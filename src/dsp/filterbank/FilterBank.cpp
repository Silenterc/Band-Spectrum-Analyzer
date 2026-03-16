//
// Created by Lukáš Zima on 16.03.2026.
//

#include "FilterBank.h"

namespace Analyzer {
    void FilterBank::prepare(double newSampleRate, std::shared_ptr<const std::vector<BandInfo>> newBandInfo) {
        bandPassFilters.clear();
        sumPowers.clear();
        peakPowers.clear();
        sampleRate = newSampleRate;
        bandInfo = std::move(newBandInfo);
        prepareFilters();
    }

    void FilterBank::reset() {
        sampleRate = 0.0;
        bandInfo = nullptr;
        bandPassFilters.clear();
        sumPowers.clear();
        peakPowers.clear();
    }

    void FilterBank::processBlock(const SignalView& signalView, std::vector<BandMeasurements>& outputMeasurements) {
        if (bandInfo == nullptr || signalView.data == nullptr || signalView.numSamples == 0) {
            return;
        }

        const auto bandAmount = bandInfo->size();

        constexpr size_t groupWidth = 4;
        const auto zero = SimdFloat::expand(0.0f);

        for (auto& sumPower : sumPowers) {
            sumPower = zero;
        }

        for (auto& peakPower : peakPowers) {
            peakPower = zero;
        }

        for (size_t bandIndex = 0; bandIndex < bandAmount; ++bandIndex) {
            outputMeasurements[bandIndex] = {};
        }

        for (size_t i = 0; i < signalView.numSamples; ++i) {
            const float sample = signalView.data[i];
            for (size_t filterIndex = 0; filterIndex < bandPassFilters.size(); ++filterIndex) {
                const auto output = bandPassFilters[filterIndex].process(sample);
                const auto power = output * output;
                sumPowers[filterIndex] += power;
                peakPowers[filterIndex] = juce::jmax(peakPowers[filterIndex], power);
            }
        }

        for (size_t filterIndex = 0; filterIndex < bandPassFilters.size(); ++filterIndex) {
            alignas(16) float summedPowers[groupWidth];
            alignas(16) float peakPowerValues[groupWidth];
            sumPowers[filterIndex].copyToRawArray(summedPowers);
            peakPowers[filterIndex].copyToRawArray(peakPowerValues);

            for (size_t lane = 0; lane < groupWidth; ++lane) {
                const size_t bandIndex = filterIndex * groupWidth + lane;

                if (bandIndex >= bandAmount) {
                    break;
                }

                auto& measurements = outputMeasurements[bandIndex];
                measurements.peakPower = peakPowerValues[lane];
                measurements.sumPower = static_cast<double>(summedPowers[lane]);
                measurements.numSamples = static_cast<int>(signalView.numSamples);
            }
        }
    }

    void FilterBank::prepareFilters() {
        bandPassFilters.clear();
        sumPowers.clear();
        peakPowers.clear();

        if (bandInfo == nullptr) {
            return;
        }

        const auto bandAmount = bandInfo->size();
        // SIMD of 4
        constexpr size_t groupWidth = 4;
        const size_t numGroups = (bandAmount + groupWidth - 1) / groupWidth;
        bandPassFilters.reserve(numGroups);
        sumPowers.resize(numGroups);
        peakPowers.resize(numGroups);

        for (size_t g = 0; g < numGroups; ++g) {
            // Temporary scalar coeffs for 4 lanes
            float a1[groupWidth] = { 0, 0, 0, 0 };
            float a2[groupWidth] = { 0, 0, 0, 0 };
            float b0[groupWidth] = { 0, 0, 0, 0 };
            float b1[groupWidth] = { 0, 0, 0, 0 };
            float b2[groupWidth] = { 0, 0, 0, 0 };

            for (size_t lane = 0; lane < groupWidth; ++lane) {
                const size_t index = g * groupWidth + lane;

                if (index >= bandAmount) {
                    // Apply padding to the rest
                    a1[lane] = a2[lane] = 0.0f;
                    b0[lane] = b1[lane] = b2[lane] = 0.0f;
                    continue;
                }

                const auto& band = (*bandInfo)[index];

                const float w0 = 2 * juce::MathConstants<float>::pi * band.centerHz / static_cast<float>(sampleRate);
                // In octaves
                float bandwidth = std::log2f(band.highHz / band.lowHz);
                float sinW0 = juce::dsp::FastMathApproximations::sin(w0);
                float cosW0 = juce::dsp::FastMathApproximations::cos(w0);
                float alpha = sinW0 * juce::dsp::FastMathApproximations::sinh((std::logf(2) / 2) * bandwidth * (w0 / sinW0));

                // RBJ bandpass (constant peak gain) coeffs unnormalized
                float bb0 = alpha;
                float bb1 = 0.f;
                float bb2 = -alpha;
                const float aa0 = 1.0f + alpha;
                float aa1 = -2.0f * cosW0;
                float aa2 = 1.0f - alpha;

                const float invA0 = 1.0f / aa0;

                // Normalize by a0
                bb0 *= invA0; bb1 *= invA0; bb2 *= invA0;
                aa1 *= invA0; aa2 *= invA0;

                a1[lane] = aa1; a2[lane] = aa2;
                b0[lane] = bb0; b1[lane] = bb1; b2[lane] = bb2;
            }
            BandPassFilterParams packed;
            packed.a1 = juce::dsp::SIMDRegister<float>::fromNative({a1[0], a1[1], a1[2], a1[3]});
            packed.a2 = juce::dsp::SIMDRegister<float>::fromNative({a2[0], a2[1], a2[2], a2[3]});
            packed.b0 = juce::dsp::SIMDRegister<float>::fromNative({b0[0], b0[1], b0[2], b0[3]});
            packed.b1 = juce::dsp::SIMDRegister<float>::fromNative({b1[0], b1[1], b1[2], b1[3]});
            packed.b2 = juce::dsp::SIMDRegister<float>::fromNative({b2[0], b2[1], b2[2], b2[3]});

            SIMDBPFilter filter;
            filter.prepare(packed);
            bandPassFilters.push_back(filter);
        }
    }
}
