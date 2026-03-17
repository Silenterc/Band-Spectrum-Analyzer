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

        filterBank.prepare(sampleRate,
                           std::move(bandInfo),
                           spec.secondarySignal.has_value() ? FilterBank::Mode::stereoAverage : FilterBank::Mode::singleLane);
    }

    void AnalysisGroupProcessor::reset() {
        filterBank.reset();
        clearOutputMeasurements();
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

    void AnalysisGroupProcessor::writeRawTraces(std::vector<RawTrace>& destination, const size_t startIndex) const {
        auto& trace = destination[startIndex];
        trace.kind = spec.kind;

        if (trace.measurements.size() != outputMeasurements.size())
            trace.measurements.resize(outputMeasurements.size());

        std::copy(outputMeasurements.begin(), outputMeasurements.end(), trace.measurements.begin());
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
}
