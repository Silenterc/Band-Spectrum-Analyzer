//
// Created by Lukáš Zima on 16.03.2026.
//

#include "FilterBank.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace Analyzer {
    void FilterBank::prepare(double newSampleRate,
                             std::shared_ptr<const std::vector<BandInfo>> newBandInfo,
                             const Mode modeToUse) {
        sampleRate = newSampleRate;
        bandInfo = std::move(newBandInfo);
        mode = modeToUse;
        prepareFilters();
        reset();
    }

    void FilterBank::reset() {
        for (auto& filter : primaryFilters)
            filter.reset();

        for (auto& filter : secondaryFilters)
            filter.reset();

        const auto zero = SimdFloat::expand(0.0f);
        for (auto& sumPower : sumPowers)
            sumPower = zero;

        for (auto& peakPower : peakPowers)
            peakPower = zero;
    }

    void FilterBank::processBlock(const SignalView& primarySignalView,
                                  const SignalView* secondarySignalView,
                                  std::vector<BandMeasurements>& outputMeasurements) {
        if (bandInfo == nullptr || primarySignalView.data == nullptr || primarySignalView.numSamples == 0)
            return;

        if (mode == Mode::stereoAverage
            && (secondarySignalView == nullptr || secondarySignalView->data == nullptr || secondarySignalView->numSamples == 0)) {
            return;
        }

        const auto bandAmount = bandInfo->size();
        if (outputMeasurements.size() < bandAmount)
            return;

        for (size_t bandIndex = 0; bandIndex < bandAmount; ++bandIndex)
            outputMeasurements[bandIndex] = {};

        switch (mode) {
            case Mode::singleLane:
                processBlockSingleLane(primarySignalView, outputMeasurements);
                return;
            case Mode::stereoAverage:
                processBlockStereoAverage(primarySignalView, *secondarySignalView, outputMeasurements);
                return;
        }
    }

    void FilterBank::prepareFilters() {
        primaryFilters.clear();
        secondaryFilters.clear();
        sumPowers.clear();
        peakPowers.clear();

        if (bandInfo == nullptr)
            return;

        const auto bandAmount = bandInfo->size();
        const size_t numGroups = (bandAmount + groupWidth - 1) / groupWidth;
        primaryFilters.reserve(numGroups);
        if (mode == Mode::stereoAverage)
            secondaryFilters.reserve(numGroups);

        sumPowers.resize(numGroups);
        peakPowers.resize(numGroups);

        for (size_t groupIndex = 0; groupIndex < numGroups; ++groupIndex) {
            // Each SIMD lane maps to one logical band inside this packed group.
            alignas(sizeof(SimdFloat)) std::array<float, groupWidth> a1{};
            alignas(sizeof(SimdFloat)) std::array<float, groupWidth> a2{};
            alignas(sizeof(SimdFloat)) std::array<float, groupWidth> b0{};
            alignas(sizeof(SimdFloat)) std::array<float, groupWidth> b1{};
            alignas(sizeof(SimdFloat)) std::array<float, groupWidth> b2{};

            for (size_t lane = 0; lane < groupWidth; ++lane) {
                const size_t bandIndex = groupIndex * groupWidth + lane;
                if (bandIndex >= bandAmount)
                    continue;

                const auto& band = (*bandInfo)[bandIndex];
                const auto w0 = 2.0f * juce::MathConstants<float>::pi * band.centerHz / static_cast<float>(sampleRate);
                const auto bandwidth = std::log2(band.highHz / band.lowHz);
                // We want a narrower band for the analyzer
                const auto sinW0 = juce::dsp::FastMathApproximations::sin(w0);
                const auto cosW0 = juce::dsp::FastMathApproximations::cos(w0);
                const auto alpha = sinW0 * juce::dsp::FastMathApproximations::sinh((std::log(2.0f) * 0.5f)
                                                                                   * bandwidth * (w0 / sinW0));

                auto bb0 = alpha;
                auto bb1 = 0.0f;
                auto bb2 = -alpha;
                const auto aa0 = 1.0f + alpha;
                auto aa1 = -2.0f * cosW0;
                auto aa2 = 1.0f - alpha;
                const auto invA0 = 1.0f / aa0;

                bb0 *= invA0;
                bb1 *= invA0;
                bb2 *= invA0;
                aa1 *= invA0;
                aa2 *= invA0;

                a1[lane] = aa1;
                a2[lane] = aa2;
                b0[lane] = bb0;
                b1[lane] = bb1;
                b2[lane] = bb2;
            }

            BandPassFilterParams packed;
            packed.a1 = SimdFloat::fromRawArray(a1.data());
            packed.a2 = SimdFloat::fromRawArray(a2.data());
            packed.b0 = SimdFloat::fromRawArray(b0.data());
            packed.b1 = SimdFloat::fromRawArray(b1.data());
            packed.b2 = SimdFloat::fromRawArray(b2.data());

            SIMDBPFilter primaryFilter;
            primaryFilter.prepare(packed);
            primaryFilters.push_back(primaryFilter);

            if (mode == Mode::stereoAverage) {
                SIMDBPFilter secondaryFilter;
                secondaryFilter.prepare(packed);
                secondaryFilters.push_back(secondaryFilter);
            }
        }
    }

    void FilterBank::processBlockSingleLane(const SignalView& signalView,
                                            std::vector<BandMeasurements>& outputMeasurements) {
        clearPowerAccumulators();

        auto* filters = primaryFilters.data();
        auto* sums = sumPowers.data();
        auto* peaks = peakPowers.data();
        const auto filterCount = primaryFilters.size();

        for (size_t sampleIndex = 0; sampleIndex < signalView.numSamples; ++sampleIndex) {
            // Broadcast the scalar input sample into all lanes so one pass updates a whole SIMD group.
            const SimdFloat input(signalView.data[sampleIndex]);

            for (size_t filterIndex = 0; filterIndex < filterCount; ++filterIndex) {
                const auto output = filters[filterIndex].process(input);
                const auto power = output * output;
                sums[filterIndex] += power;
                peaks[filterIndex] = juce::jmax(peaks[filterIndex], power);
            }
        }

        writeMeasurements(bandInfo->size(), signalView.numSamples, outputMeasurements);
    }

    void FilterBank::processBlockStereoAverage(const SignalView& primarySignalView,
                                               const SignalView& secondarySignalView,
                                               std::vector<BandMeasurements>& outputMeasurements) {
        clearPowerAccumulators();

        auto* primary = primaryFilters.data();
        auto* secondary = secondaryFilters.data();
        auto* sums = sumPowers.data();
        auto* peaks = peakPowers.data();
        const auto filterCount = primaryFilters.size();
        const auto numSamples = std::min(primarySignalView.numSamples, secondarySignalView.numSamples);
        const auto half = SimdFloat::expand(0.5f);

        for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
            const SimdFloat primaryInput(primarySignalView.data[sampleIndex]);
            const SimdFloat secondaryInput(secondarySignalView.data[sampleIndex]);

            for (size_t filterIndex = 0; filterIndex < filterCount; ++filterIndex) {
                const auto primaryOutput = primary[filterIndex].process(primaryInput);
                const auto secondaryOutput = secondary[filterIndex].process(secondaryInput);
                const auto power = half * (primaryOutput * primaryOutput + secondaryOutput * secondaryOutput);
                sums[filterIndex] += power;
                peaks[filterIndex] = juce::jmax(peaks[filterIndex], power);
            }
        }

        writeMeasurements(bandInfo->size(), numSamples, outputMeasurements);
    }

    void FilterBank::writeMeasurements(const size_t bandAmount,
                                       const size_t numSamples,
                                       std::vector<BandMeasurements>& outputMeasurements) const {
        for (size_t filterIndex = 0; filterIndex < primaryFilters.size(); ++filterIndex) {
            alignas(sizeof(SimdFloat)) std::array<float, groupWidth> summedPowers{};
            alignas(sizeof(SimdFloat)) std::array<float, groupWidth> peakPowerValues{};
            sumPowers[filterIndex].copyToRawArray(summedPowers.data());
            peakPowers[filterIndex].copyToRawArray(peakPowerValues.data());

            // Convert packed SIMD lanes back into the scalar storage consumed by the rest of the analyzer.
            for (size_t lane = 0; lane < groupWidth; ++lane) {
                const size_t bandIndex = filterIndex * groupWidth + lane;
                if (bandIndex >= bandAmount)
                    break;

                auto& measurements = outputMeasurements[bandIndex];
                measurements.peakPower = peakPowerValues[lane];
                measurements.rmsHopSumPower = static_cast<double>(summedPowers[lane]);
                measurements.rmsHopNumSamples = static_cast<int>(numSamples);
            }
        }
    }

    void FilterBank::clearPowerAccumulators() {
        const auto zero = SimdFloat::expand(0.0f);

        for (auto& sumPower : sumPowers)
            sumPower = zero;

        for (auto& peakPower : peakPowers)
            peakPower = zero;
    }
}
