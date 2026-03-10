#include "AnalysisGroupProcessor.h"

#include <algorithm>

namespace Analyzer {
    AnalysisGroupProcessor::AnalysisGroupProcessor(AnalysisGroupSpec specToUse)
        : spec(std::move(specToUse)) {
    }

    void AnalysisGroupProcessor::prepare(double sampleRate, int maximumBlockSize,
                                         std::shared_ptr<const std::vector<BandInfo>> bandInfo) {
        juce::dsp::ProcessSpec processSpec{};
        processSpec.sampleRate = sampleRate;
        processSpec.maximumBlockSize = static_cast<juce::uint32>(maximumBlockSize);
        processSpec.numChannels = static_cast<juce::uint32>(std::max<size_t>(spec.lanes.size(), 1));

        const auto bandCount = bandInfo != nullptr ? bandInfo->size() : 0;
        bands.resize(bandCount);
        outputMeasurements.resize(spec.outputs.size());

        laneData.assign(spec.lanes.size(), nullptr);
        lanePowers.assign(spec.lanes.size(), 0.0f);
        peakPowers.assign(spec.outputs.size(), 0.0f);
        sumPowers.assign(spec.outputs.size(), 0.0);

        for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
            auto &band = bands[bandIndex];
            band.info = (*bandInfo)[bandIndex];
            const auto bandwidth = std::max(band.info.highHz - band.info.lowHz, 1.0f);
            // Rough q from the band width. Good enough until we calibrate this more carefully
            // Higher resonance means a narrower, more focused band around the center freq
            const auto resonance = juce::jlimit(0.1f, 10.0f, band.info.centerHz / bandwidth);

            band.filter.prepare(processSpec);
            band.filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
            band.filter.setCutoffFrequency(band.info.centerHz);
            band.filter.setResonance(resonance);
            band.filter.reset();
        }

        for (auto &measurements: outputMeasurements)
            measurements.assign(bandCount, {});
    }

    void AnalysisGroupProcessor::reset() {
        for (auto &band: bands)
            band.filter.reset();

        for (auto &measurements: outputMeasurements)
            std::fill(measurements.begin(), measurements.end(), BandMeasurements{});
    }

    void AnalysisGroupProcessor::process(const SourceSet &sources) {
        auto numSamples = size_t{0};
        for (size_t laneIndex = 0; laneIndex < spec.lanes.size(); ++laneIndex) {
            const auto signalView = selectSignalView(sources, spec.sourceFamily, spec.lanes[laneIndex].signal);
            laneData[laneIndex] = signalView.data;
            numSamples = signalView.numSamples;
        }

        for (size_t bandIndex = 0; bandIndex < bands.size(); ++bandIndex) {
            auto &band = bands[bandIndex];
            std::fill(peakPowers.begin(), peakPowers.end(), 0.0f);
            std::fill(sumPowers.begin(), sumPowers.end(), 0.0);

            for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                for (size_t laneIndex = 0; laneIndex < laneData.size(); ++laneIndex) {
                    const auto filteredSample = band.filter.processSample(static_cast<int>(laneIndex),
                                                                          laneData[laneIndex][sampleIndex]);
                    lanePowers[laneIndex] = filteredSample * filteredSample;
                }

                for (size_t outputIndex = 0; outputIndex < spec.outputs.size(); ++outputIndex) {
                    const auto samplePower = computeOutputSamplePower(spec.outputs[outputIndex], lanePowers);
                    if (samplePower > peakPowers[outputIndex])
                        peakPowers[outputIndex] = samplePower;
                    sumPowers[outputIndex] += static_cast<double>(samplePower);
                }
            }

            for (size_t outputIndex = 0; outputIndex < spec.outputs.size(); ++outputIndex) {
                auto &measurements = outputMeasurements[outputIndex][bandIndex];
                measurements.peakPower = peakPowers[outputIndex];
                measurements.sumPower = sumPowers[outputIndex];
                measurements.numSamples = static_cast<int>(numSamples);
            }
        }
    }

    void AnalysisGroupProcessor::writeRawTraces(std::vector<RawTrace> &destination, size_t startIndex) const {
        for (size_t outputIndex = 0; outputIndex < spec.outputs.size(); ++outputIndex) {
            auto &trace = destination[startIndex + outputIndex];
            trace.kind = spec.outputs[outputIndex].kind;

            if (trace.measurements.size() != outputMeasurements[outputIndex].size())
                trace.measurements.resize(outputMeasurements[outputIndex].size());

            std::copy(outputMeasurements[outputIndex].begin(), outputMeasurements[outputIndex].end(),
                      trace.measurements.begin());
        }
    }

    size_t AnalysisGroupProcessor::getOutputCount() const {
        return spec.outputs.size();
    }

    SignalView AnalysisGroupProcessor::selectSignalView(const SourceSet &sources, SourceFamily sourceFamily,
                                                        DerivedSignal signal) const {
        if (sourceFamily == SourceFamily::mainInput) {
            switch (signal) {
                case DerivedSignal::mid:
                    return sources.mainMid;
                case DerivedSignal::left:
                    return sources.mainLeft;
                case DerivedSignal::right:
                    return sources.mainRight;
                case DerivedSignal::side:
                    return sources.mainSide;
            }
        }

        switch (signal) {
            case DerivedSignal::mid:
                return sources.sidechainMid;
            case DerivedSignal::left:
                return sources.sidechainLeft;
            case DerivedSignal::right:
                return sources.sidechainRight;
            case DerivedSignal::side:
                return sources.sidechainSide;
        }

        return SignalView();
    }

    float AnalysisGroupProcessor::computeOutputSamplePower(const AnalysisOutputSpec &outputSpec,
                                                           const std::vector<float> &currentLanePowers) const {
        if (outputSpec.laneIndices.empty())
            return 0.0f;

        if (outputSpec.mixMode == OutputMixMode::singleLane)
            return currentLanePowers[outputSpec.laneIndices.front()];

        auto combinedPower = 0.0f;
        for (auto laneIndex: outputSpec.laneIndices)
            combinedPower += currentLanePowers[laneIndex];

        return combinedPower / static_cast<float>(outputSpec.laneIndices.size());
    }
}
