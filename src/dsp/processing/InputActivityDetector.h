#pragma once

#include <algorithm>

#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/core/AnalyzerConstants.h"

namespace Analyzer {
    /**
     * Tracks whether recent input energy is high enough to justify analyzer processing.
     */
    class InputActivityDetector final {
    public:
        void prepare(const double newSampleRate) {
            sampleRate = newSampleRate;
            reset();
        }

        void reset() {
            state = State::idle;
            lastPeakAbs = 0.0f;
            silentSamplesRemaining = 0;
        }

        void update(const juce::AudioBuffer<float> &mainBuffer,
                    const juce::AudioBuffer<float> *sidechainBuffer) {
            lastPeakAbs = std::max(getPeakAbs(mainBuffer), getPeakAbs(sidechainBuffer));

            if (lastPeakAbs >= Constants::activityActivateThresholdGain) {
                state = State::active;
                silentSamplesRemaining = getSilentHoldSamples();
                return;
            }

            if (state == State::idle)
                return;

            if (lastPeakAbs >= Constants::activityDeactivateThresholdGain) {
                state = State::active;
                silentSamplesRemaining = getSilentHoldSamples();
                return;
            }

            silentSamplesRemaining = std::max(0, silentSamplesRemaining - mainBuffer.getNumSamples());
            state = silentSamplesRemaining > 0 ? State::silentHold : State::idle;
        }

        [[nodiscard]] bool shouldProcess() const {
            return state != State::idle;
        }

        [[nodiscard]] bool hasRecentSignal() const {
            return state == State::active;
        }

    private:
        enum class State {
            active,
            silentHold,
            idle
        };

        static float getPeakAbs(const juce::AudioBuffer<float> *buffer) {
            if (buffer == nullptr)
                return 0.0f;

            return getPeakAbs(*buffer);
        }

        static float getPeakAbs(const juce::AudioBuffer<float> &buffer) {
            float peak = 0.0f;

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                peak = std::max(peak, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));

            return peak;
        }

        [[nodiscard]] int getSilentHoldSamples() const {
            return static_cast<int>(sampleRate * Constants::activitySilenceHoldMs * 0.001);
        }

        State state = State::idle;
        double sampleRate = Constants::defaultSampleRateHz;
        float lastPeakAbs = 0.0f;
        int silentSamplesRemaining = 0;
    };
}
