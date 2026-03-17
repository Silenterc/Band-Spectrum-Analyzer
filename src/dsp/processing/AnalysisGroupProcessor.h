#pragma once

#include <array>
#include <memory>
#include <vector>

#include "../core/AnalyzerData.h"
#include "../filterbank/FilterBank.h"
#include "../planning/AnalysisPlanBuilder.h"
#include "../sources/AnalysisSourceBuilder.h"

namespace Analyzer {
    /**
     * Executes one analysis group over a block using its own filter bank state.
     */
    class AnalysisGroupProcessor {
    public:
        AnalysisGroupProcessor() = default;
        explicit AnalysisGroupProcessor(AnalysisGroupSpec specToUse);

        void prepare(double sampleRate, int maximumBlockSize,
                     std::shared_ptr<const std::vector<BandInfo>> bandInfo);

        void reset();

        void process(const SourceSet& sources);
        void accumulateCurrentSlice(size_t frameSlotIndex);
        void clearAccumulatedFrame(size_t frameSlotIndex);

        void writeRawTraces(std::vector<RawTrace>& destination, size_t startIndex, size_t frameSlotIndex) const;

        size_t getOutputCount() const;

    private:
        SignalView selectSignalView(const SourceSet& sources, SourceFamily sourceFamily,
                                    DerivedSignal signal) const;
        void clearOutputMeasurements();
        void clearAccumulatedMeasurements();

        AnalysisGroupSpec spec;
        FilterBank filterBank;
        // Scratch measurements for the current sliced input segment.
        std::vector<BandMeasurements> outputMeasurements;
        // Two overlapping fixed-size analysis frames are active at most with the current 50% hop.
        std::array<std::vector<BandMeasurements>, 2> accumulatedMeasurements;
    };
}
