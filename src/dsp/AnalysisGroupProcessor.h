#pragma once

#include <memory>
#include <vector>

#include <juce_dsp/juce_dsp.h>

#include "AnalysisPlanBuilder.h"
#include "AnalysisSourceBuilder.h"
#include "AnalyzerData.h"

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

        float computeOutputSamplePower(const AnalysisOutputSpec &outputSpec,
                                       const std::vector<float> &lanePowers) const;

        AnalysisGroupSpec spec;
        std::vector<BandState> bands;
        std::vector<std::vector<BandMeasurements>> outputMeasurements;

        // Reused process scratch to avoid per-block allocations in the hot path
        std::vector<const float *> laneData;
        std::vector<float> lanePowers;
        std::vector<float> peakPowers;
        std::vector<double> sumPowers;
    };
}
