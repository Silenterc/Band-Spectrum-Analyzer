#include "AnalysisGroupProcessor.h"

#include <algorithm>

namespace Analyzer {
    AnalysisGroupProcessor::AnalysisGroupProcessor(AnalysisGroupSpec specToUse)
        : spec(std::move(specToUse)),
          processingShape(spec.secondarySignal.has_value() ? ProcessingShape::stereoAverage : ProcessingShape::singleLane) {
    }

    void AnalysisGroupProcessor::prepare(double sampleRate, int maximumBlockSize,
                                         std::shared_ptr<const std::vector<BandInfo>> bandInfo) {
        juce::dsp::ProcessSpec processSpec{};
        processSpec.sampleRate = sampleRate;
        processSpec.maximumBlockSize = static_cast<juce::uint32>(maximumBlockSize);
        processSpec.numChannels = processingShape == ProcessingShape::stereoAverage ? 2u : 1u;

        const auto bandCount = bandInfo != nullptr ? bandInfo->size() : 0;
        bands.resize(bandCount);
        outputMeasurements.assign(bandCount, {});
        laneData = {nullptr, nullptr};

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
    }

    void AnalysisGroupProcessor::reset() {
        for (auto &band: bands)
            band.filter.reset();

        clearOutputMeasurements();
    }

    void AnalysisGroupProcessor::process(const SourceSet &sources) {
        const auto numSamples = updateLaneData(sources);

        if (numSamples == 0) {
            clearOutputMeasurements();
            return;
        }

        switch (processingShape) {
            case ProcessingShape::singleLane:
                processSingleLane(numSamples);
                return;
            case ProcessingShape::stereoAverage:
                processStereoAverage(numSamples);
                return;
        }
    }

    void AnalysisGroupProcessor::writeRawTraces(std::vector<RawTrace> &destination, size_t startIndex) const {
        auto &trace = destination[startIndex];
        trace.kind = spec.kind;

        if (trace.measurements.size() != outputMeasurements.size())
            trace.measurements.resize(outputMeasurements.size());

        std::copy(outputMeasurements.begin(), outputMeasurements.end(), trace.measurements.begin());
    }

    size_t AnalysisGroupProcessor::getOutputCount() const {
        return 1;
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

    size_t AnalysisGroupProcessor::updateLaneData(const SourceSet &sources) {
        const auto primarySignalView = selectSignalView(sources, spec.sourceFamily, spec.primarySignal);
        laneData[0] = primarySignalView.data;
        laneData[1] = nullptr;

        if (laneData[0] == nullptr)
            return 0;

        if (processingShape != ProcessingShape::stereoAverage || !spec.secondarySignal.has_value())
            return primarySignalView.numSamples;

        const auto secondarySignalView = selectSignalView(sources, spec.sourceFamily, *spec.secondarySignal);
        laneData[1] = secondarySignalView.data;
        if (laneData[1] == nullptr)
            return 0;

        return std::min(primarySignalView.numSamples, secondarySignalView.numSamples);
    }

    void AnalysisGroupProcessor::clearOutputMeasurements() {
        std::fill(outputMeasurements.begin(), outputMeasurements.end(), BandMeasurements{});
    }

    void AnalysisGroupProcessor::processSingleLane(const size_t numSamples) {
        if (laneData[0] == nullptr) {
            clearOutputMeasurements();
            return;
        }

        const auto *lane = laneData[0];

        for (size_t bandIndex = 0; bandIndex < bands.size(); ++bandIndex) {
            auto &band = bands[bandIndex];
            auto peakPower = 0.0f;
            auto sumPower = 0.0;

            for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                const auto filteredSample = band.filter.processSample(0, lane[sampleIndex]);
                const auto power = filteredSample * filteredSample;
                peakPower = std::max(peakPower, power);
                sumPower += static_cast<double>(power);
            }

            auto &measurements = outputMeasurements[bandIndex];
            measurements.peakPower = peakPower;
            measurements.sumPower = sumPower;
            measurements.numSamples = static_cast<int>(numSamples);
        }
    }

    void AnalysisGroupProcessor::processStereoAverage(const size_t numSamples) {
        if (laneData[0] == nullptr || laneData[1] == nullptr) {
            clearOutputMeasurements();
            return;
        }

        const auto *leftLane = laneData[0];
        const auto *rightLane = laneData[1];

        for (size_t bandIndex = 0; bandIndex < bands.size(); ++bandIndex) {
            auto &band = bands[bandIndex];
            auto peakPower = 0.0f;
            auto sumPower = 0.0;

            for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                const auto leftFiltered = band.filter.processSample(0, leftLane[sampleIndex]);
                const auto rightFiltered = band.filter.processSample(1, rightLane[sampleIndex]);
                const auto power = 0.5f * (leftFiltered * leftFiltered + rightFiltered * rightFiltered);
                peakPower = std::max(peakPower, power);
                sumPower += static_cast<double>(power);
            }

            auto &measurements = outputMeasurements[bandIndex];
            measurements.peakPower = peakPower;
            measurements.sumPower = sumPower;
            measurements.numSamples = static_cast<int>(numSamples);
        }
    }
}
