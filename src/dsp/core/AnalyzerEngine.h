#pragma once

#include <atomic>
#include <array>
#include <memory>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>

#include "../planning/AnalysisPlanBuilder.h"
#include "../processing/AnalysisGroupProcessor.h"
#include "../processing/InputActivityDetector.h"
#include "../sources/AnalysisSourceBuilder.h"
#include "../util/TripleBuffer.h"
#include "display/analyzer/contracts/AnalyzerPublishedTracesView.h"
#include "AnalyzerConstants.h"
#include "AnalyzerData.h"
#include "EngineParameterState.h"

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
         * Returns a non-owning view of the latest published raw traces.
         */
        AnalyzerPublishedTracesView readPublishedTraces() const;

        /**
         * Returns whether the recent input history still counts as active.
         */
        bool hasRecentSignal() const;

    private:
        struct FrameSlotState {
            bool active = false;
            size_t fillSamples = 0;
        };

        static SignalView sliceSignalView(const SignalView &view, size_t offset, size_t numSamples);
        static SourceSet sliceSourceSet(const SourceSet &sourceSet, size_t offset, size_t numSamples);

        /**
         * Rebuilds the fractional-octave band layout
         */
        void rebuildBands();

        /**
         * Rebuilds the active group processors for the current parameter state
         */
        void rebuildProcessors();

        /**
         * Returns how many bands per octave the current mode should use
         */
        int getBandsPerOctave() const;

        /**
         * Publishes one accumulated analysis frame, optionally after resetting processor state.
         */
        void publishProcessorState(bool resetProcessors, size_t frameSlotIndex = 0);

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
        // Tracks whether recent input energy justifies analyzer processing
        InputActivityDetector inputActivityDetector;
        // Active modular analysis processors
        std::vector<AnalysisGroupProcessor> processors;
        // Total number of published traces produced by the active processors
        size_t publishedTraceCount = 0;
        // Published raw traces for the UI, 1 W x 1 R threads
        mutable TripleBuffer<std::vector<RawTrace> > traces;
        // Two overlapping frame slots are enough for the current 50% hop.
        std::array<FrameSlotState, 2> frameSlots{};
        // Slot index that should start the next analysis frame at the next hop boundary.
        size_t nextFrameSlotToStart = 1;
        // Number of samples remaining until the next hop boundary.
        size_t samplesUntilNextFrameStart = Constants::analysisHopSamples;
        // Runtime flag exposed to the UI so it can idle once the display has decayed to floor
        std::atomic<bool> recentSignalActive { false };
        // Ensures silence is published only once per inactive period
        bool hasPublishedSilenceWhileInactive = false;
    };
}
