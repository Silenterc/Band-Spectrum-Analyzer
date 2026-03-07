#pragma once

#include <array>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include "ParameterState.h"

namespace Analyzer {
    /**
     * Frequency bounds for a single analyzer band
     */
    struct BandInfo {
        // Lower edge of the band in Hz
        float lowHz = 0.0f;
        // Geometric center of the band in Hz
        float centerHz = 0.0f;
        // Upper edge of the band in Hz
        float highHz = 0.0f;
    };

    /**
     * Latest analyzer output for all bands
     * You can get related band info (freq etc.) by calling getBandInfo()
     */
    struct Frame {
        // RMS level per band in dB
        std::vector<float> rmsDb;
        // Peak level per band in dB
        std::vector<float> peakDb;
        // Peak hold level per band in dB
        std::vector<float> holdDb;
    };

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
         * Returns the current band layout
         */
        const std::vector<BandInfo> &getBandInfo() const;

        /**
         * Returns the latest computed analyzer frame
         */
        const Frame &getLatestFrame() const;

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
         * Builds the mono summed input stream used by the current analyzer path
         */
        void updateSummedAnalysisInput(const juce::AudioBuffer<float> &buffer);

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

        // Temporary mono input used by the summed analysis path
        std::vector<float> summedAnalysisInput;
        // Static band metadata for the current layout
        std::vector<BandInfo> bandInfo;
        // Per-band filters and meter state
        std::vector<BandState> bands;
        // Latest frame exposed to the UI
        Frame latestFrame;

        // How fast the hold falls once it does, dB per second
        float holdDecayDbPerSecond = 12.0f;
    };
}
