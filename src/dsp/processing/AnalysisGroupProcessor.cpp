#include "AnalysisGroupProcessor.h"

#include <algorithm>

namespace Analyzer {
    AnalysisGroupProcessor::AnalysisGroupProcessor(AnalysisGroupSpec specToUse)
        : spec(std::move(specToUse)) {
    }

    void AnalysisGroupProcessor::prepare(double sampleRate, int maximumBlockSize,
                                         std::shared_ptr<const std::vector<BandInfo>> bandInfo) {
        juce::ignoreUnused(maximumBlockSize);

        const auto bandCount = bandInfo != nullptr ? bandInfo->size() : 0;
        outputMeasurements.assign(bandCount, {});
        for (auto &frameMeasurements: accumulatedMeasurements)
            frameMeasurements.assign(bandCount, {});

        filterBank.prepare(sampleRate,
                           std::move(bandInfo),
                           spec.secondarySignal.has_value() ? FilterBank::Mode::stereoAverage : FilterBank::Mode::singleLane);
    }

    void AnalysisGroupProcessor::reset() {
        filterBank.reset();
        clearOutputMeasurements();
        clearAccumulatedMeasurements();
    }

    void AnalysisGroupProcessor::process(const SourceSet& sources) {
        const auto primarySignalView = selectSignalView(sources, spec.sourceFamily, spec.primarySignal);
        if (primarySignalView.data == nullptr || primarySignalView.numSamples == 0) {
            clearOutputMeasurements();
            return;
        }

        if (!spec.secondarySignal.has_value()) {
            filterBank.processBlock(primarySignalView, nullptr, outputMeasurements);
            return;
        }

        const auto secondarySignalView = selectSignalView(sources, spec.sourceFamily, *spec.secondarySignal);
        if (secondarySignalView.data == nullptr || secondarySignalView.numSamples == 0) {
            clearOutputMeasurements();
            return;
        }

        filterBank.processBlock(primarySignalView, &secondarySignalView, outputMeasurements);
    }

    void AnalysisGroupProcessor::accumulateCurrentSlice(const size_t frameSlotIndex) {
        jassert(frameSlotIndex < accumulatedMeasurements.size());
        auto &frameMeasurements = accumulatedMeasurements[frameSlotIndex];
        if (frameMeasurements.size() != outputMeasurements.size())
            frameMeasurements.assign(outputMeasurements.size(), {});

        for (size_t bandIndex = 0; bandIndex < outputMeasurements.size(); ++bandIndex) {
            const auto &sliceMeasurements = outputMeasurements[bandIndex];
            auto &accumulated = frameMeasurements[bandIndex];
            accumulated.peakPower = std::max(accumulated.peakPower, sliceMeasurements.peakPower);
            accumulated.sumPower += sliceMeasurements.sumPower;
            accumulated.numSamples += sliceMeasurements.numSamples;
        }
    }

    void AnalysisGroupProcessor::clearAccumulatedFrame(const size_t frameSlotIndex) {
        jassert(frameSlotIndex < accumulatedMeasurements.size());
        std::fill(accumulatedMeasurements[frameSlotIndex].begin(),
                  accumulatedMeasurements[frameSlotIndex].end(),
                  BandMeasurements{});
    }

    void AnalysisGroupProcessor::writeRawTraces(std::vector<RawTrace>& destination,
                                                const size_t startIndex,
                                                const size_t frameSlotIndex) const {
        jassert(frameSlotIndex < accumulatedMeasurements.size());
        auto& trace = destination[startIndex];
        trace.kind = spec.kind;

        const auto &frameMeasurements = accumulatedMeasurements[frameSlotIndex];
        if (trace.measurements.size() != frameMeasurements.size())
            trace.measurements.resize(frameMeasurements.size());

        std::copy(frameMeasurements.begin(), frameMeasurements.end(), trace.measurements.begin());
    }

    size_t AnalysisGroupProcessor::getOutputCount() const {
        return 1;
    }

    SignalView AnalysisGroupProcessor::selectSignalView(const SourceSet& sources, const SourceFamily sourceFamily,
                                                        const DerivedSignal signal) const {
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

    void AnalysisGroupProcessor::clearOutputMeasurements() {
        std::fill(outputMeasurements.begin(), outputMeasurements.end(), BandMeasurements{});
    }

    void AnalysisGroupProcessor::clearAccumulatedMeasurements() {
        for (auto &frameMeasurements: accumulatedMeasurements) {
            std::fill(frameMeasurements.begin(), frameMeasurements.end(), BandMeasurements{});
        }
    }
}
