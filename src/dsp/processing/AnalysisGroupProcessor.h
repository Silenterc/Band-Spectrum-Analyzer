#pragma once

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

        void writeRawTraces(std::vector<RawTrace>& destination, size_t startIndex) const;

        size_t getOutputCount() const;

    private:
        SignalView selectSignalView(const SourceSet& sources, SourceFamily sourceFamily,
                                    DerivedSignal signal) const;
        void clearOutputMeasurements();

        AnalysisGroupSpec spec;
        FilterBank filterBank;
        std::vector<BandMeasurements> outputMeasurements;
    };
}
