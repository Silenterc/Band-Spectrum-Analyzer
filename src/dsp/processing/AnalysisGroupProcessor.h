#pragma once

#include <array>
#include <memory>
#include <vector>

#include <juce_dsp/juce_dsp.h>

#include "../core/AnalyzerData.h"
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

        void process(const SourceSet &sources);

        void writeRawTraces(std::vector<RawTrace> &destination, size_t startIndex) const;

        size_t getOutputCount() const;

    private:
        enum class ProcessingShape {
            singleLane,
            stereoAverage
        };

        /**
         * Runtime state for one band in the filter bank
         */
        struct BandState {
            // Static freq info for this band
            BandInfo info;
            // Bandpass filter
            juce::dsp::StateVariableTPTFilter<float> filter;
        };

        SignalView selectSignalView(const SourceSet &sources, SourceFamily sourceFamily,
                                    DerivedSignal signal) const;
        size_t updateLaneData(const SourceSet &sources);
        void clearOutputMeasurements();
        void processSingleLane(size_t numSamples);
        void processStereoAverage(size_t numSamples);

        AnalysisGroupSpec spec;
        ProcessingShape processingShape = ProcessingShape::singleLane;
        std::vector<BandState> bands;
        std::vector<BandMeasurements> outputMeasurements;

        // Reused process scratch to avoid per-block allocations in the hot path
        std::array<const float *, 2> laneData{ nullptr, nullptr };
    };
}
