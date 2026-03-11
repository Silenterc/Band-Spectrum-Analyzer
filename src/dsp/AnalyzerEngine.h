#pragma once

#include <memory>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>

#include "AnalysisGroupProcessor.h"
#include "AnalysisPlanBuilder.h"
#include "AnalysisSourceBuilder.h"
#include "AnalyzerData.h"
#include "EngineParameterState.h"
#include "TripleBuffer.h"

namespace Analyzer {
    /**
     * Owns the filter bank and produces bar data for the UI
     */
    class Engine {
    public:
        /**
         * Prepares the engine for playback and allocates its state
         */
        void prepare(double sampleRate, int maximumBlockSize);

        /**
         * Clears filter and meter state
         */
        void reset();

        /**
         * Updates the current analyzer params
         */
        void setParameters(const EngineParameterState &parameters);

        /**
         * Runs one audio block through the analyzer
         * Computes and updates the filter bank
         */
        void processBlock(const juce::AudioBuffer<float> &buffer);

        /**
         * Runs one audio block through the analyzer with an optional sidechain source
         */
        void processBlock(const juce::AudioBuffer<float> &mainBuffer,
                          const juce::AudioBuffer<float> *sidechainBuffer);

        /**
         * Returns the current immutable band layout
         */
        std::shared_ptr<const std::vector<BandInfo>> getBandInfo() const;

        /**
         * Returns the latest published raw traces for this engine
         */
        std::vector<RawTrace> getTraces() const;

    private:
        /**
         * Rebuilds the log-spaced band layout
         */
        void rebuildBands();

        /**
         * Rebuilds the active group processors for the current parameter state
         */
        void rebuildProcessors();

        /**
         * Returns how many bands the current mode should use
         */
        int getBandCount() const;

        // Current playback sample rate
        double currentSampleRate = 44100.0;
        // Largest block size we were prepared for
        int currentMaximumBlockSize = 0;
        // Whether prepare has been called
        bool isPrepared = false;

        // Latest parameter snapshot pushed into the engine
        EngineParameterState currentParameters{};

        // Static band metadata for the current layout
        std::shared_ptr<std::vector<BandInfo> > bandInfo = std::make_shared<std::vector<BandInfo> >();
        // Builds block-local source views and owns derived temp buffers
        AnalysisSourceBuilder sourceBuilder;
        // Builds the active analysis plan for the current parameter state
        AnalysisPlanBuilder planBuilder;
        // Active modular analysis processors
        std::vector<AnalysisGroupProcessor> processors;
        // Total number of published traces produced by the active processors
        size_t publishedTraceCount = 0;
        // Published raw traces for the UI, 1 W x 1 R threads
        mutable TripleBuffer<std::vector<RawTrace> > traces;
    };
}
