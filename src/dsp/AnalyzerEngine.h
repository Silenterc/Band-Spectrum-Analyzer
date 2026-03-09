#pragma once

#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include "AnalyzerData.h"
#include "ParameterState.h"
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
        void setParameters(const ParameterState &parameters);

        /**
         * Runs one audio block through the analyzer
         * Computes and updates the filter bank
         */
        void processBlock(const juce::AudioBuffer<float> &buffer);

        /**
         * Returns a stable published snapshot for one engine
         */
        EngineSnapshot getSnapshot() const;

    private:
        /**
         * Runtime state for one band in the filter bank
         */
        struct BandState {
            // Static freq info for this band
            BandInfo info;
            // Bandpass filter
            juce::dsp::StateVariableTPTFilter<float> filter;
            // RMS envelope follower for this band
            juce::dsp::BallisticsFilter<float> rmsEnvelope;
            // Peak envelope follower for this band
            juce::dsp::BallisticsFilter<float> peakEnvelope;
            // Latest RMS level in dB
            float smoothedRmsDb = ParamSpec::defaultGridMinDb;
            // Latest peak level in dB
            float smoothedPeakDb = ParamSpec::defaultGridMinDb;
            // Held peak marker value in dB
            float heldPeakDb = ParamSpec::defaultGridMinDb;
            // How much hold time is still left before the marker starts falling
            float holdTimeRemainingMs = 0.0f;
        };

        /**
         * Rebuilds the log-spaced band layout
         */
        void rebuildBands();

        /**
         * Rebuilds the bandpass filters for the current layout
         */
        void rebuildFilters();

        /**
         * Updates the mono summed input stream for the current block
         */
        void updateSummedAnalysisInput(const juce::AudioBuffer<float> &buffer);

        /**
         * Runs the summed mono analysis path for one block
         */
        void processSummedBlock(const juce::AudioBuffer<float> &buffer, float floorDb,
                                float blockDurationSeconds, float blockDurationMs);

        /**
         * Runs the "stereo" analysis path by processing L and R separately
         */
        void processStereoBlock(const juce::AudioBuffer<float> &buffer, float floorDb,
                                float blockDurationSeconds, float blockDurationMs);

        /**
         * Placeholder for the future mid/side analysis path
         */
        void processMidSideBlock(const juce::AudioBuffer<float> &buffer, float floorDb,
                                 float blockDurationSeconds, float blockDurationMs);

        /**
         * Applies common dB conversion, hold logic, and frame updates for one band
         */
        void updateBandFrame(size_t bandIndex, float rmsLinear, float peakLinear, float floorDb,
                             float blockDurationSeconds, float blockDurationMs);

        /**
         * Publishes the latest frame and band layout without locking
         */
        void publishSnapshot() const;

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
        ParameterState currentParameters{};

        // Temporary mono input vector (re)used by the summed analysis path
        std::vector<float> summedAnalysisInput;
        // Static band metadata for the current layout
        std::vector<BandInfo> bandInfo;
        // Per-band filters and meter state
        std::vector<BandState> bands;
        // Latest frame exposed to the UI
        Frame latestFrame;
        // Published analyzer snapshots for the UI, 1 W x 1 R threads
        mutable TripleBuffer<EngineSnapshot> snapshots;

        // How fast the hold falls once it does, dB per second
        float holdDecayDbPerSecond = 12.0f;
    };
}
