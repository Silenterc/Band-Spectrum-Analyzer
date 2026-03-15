#include "AnalysisGroupProcessor.h"

#include <algorithm>

namespace Analyzer {
    AnalysisGroupProcessor::AnalysisGroupProcessor(AnalysisGroupSpec specToUse)
        : spec(std::move(specToUse)) {
        determineProcessingShape();
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
        updateLaneData(sources, numSamples);

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

    void AnalysisGroupProcessor::updateLaneData(const SourceSet &sources, size_t &numSamples) {
        numSamples = 0;
        for (size_t laneIndex = 0; laneIndex < spec.lanes.size(); ++laneIndex) {
            const auto signalView = selectSignalView(sources, spec.sourceFamily, spec.lanes[laneIndex].signal);
            laneData[laneIndex] = signalView.data;
            numSamples = signalView.numSamples;
        }
    }

    void AnalysisGroupProcessor::processSingleLane(const size_t numSamples) {
        if (spec.outputs.empty() || laneData.empty() || laneData[0] == nullptr)
            return;

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

            auto &measurements = outputMeasurements[0][bandIndex];
            measurements.peakPower = peakPower;
            measurements.sumPower = sumPower;
            measurements.numSamples = static_cast<int>(numSamples);
        }
    }

    void AnalysisGroupProcessor::processStereoAverage(const size_t numSamples) {
        if (spec.outputs.empty() || laneData.size() < 2 || laneData[0] == nullptr || laneData[1] == nullptr)
            return;

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

            auto &measurements = outputMeasurements[0][bandIndex];
            measurements.peakPower = peakPower;
            measurements.sumPower = sumPower;
            measurements.numSamples = static_cast<int>(numSamples);
        }
    }

    void AnalysisGroupProcessor::determineProcessingShape() {
        processingShape = ProcessingShape::singleLane;

        if (spec.outputs.size() != 1)
            return;

        const auto &outputSpec = spec.outputs[0];

        if (outputSpec.laneIndices.size() != spec.lanes.size())
            return;

        if (spec.lanes.size() != 1 && spec.lanes.size() != 2)
            return;

        if (spec.lanes.size() == 1
            && outputSpec.mixMode == OutputMixMode::singleLane
            && outputSpec.laneIndices.size() == 1
            && outputSpec.laneIndices[0] == 0) {
            processingShape = ProcessingShape::singleLane;
            return;
        }

        if (spec.lanes.size() == 2
            && outputSpec.mixMode == OutputMixMode::averagePower
            && outputSpec.laneIndices.size() == 2
            && outputSpec.laneIndices[0] == 0
            && outputSpec.laneIndices[1] == 1) {
            processingShape = ProcessingShape::stereoAverage;
        }
    }
}
