#pragma once

#include <algorithm>

#include <juce_audio_basics/juce_audio_basics.h>

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

            if (lastPeakAbs >= activateThresholdGain) {
                state = State::active;
                silentSamplesRemaining = getSilentHoldSamples();
                return;
            }

            if (state == State::idle)
                return;

            if (lastPeakAbs >= deactivateThresholdGain) {
                state = State::active;
                silentSamplesRemaining = getSilentHoldSamples();
                return;
            }

            silentSamplesRemaining = std::max(0, silentSamplesRemaining - mainBuffer.getNumSamples());
            state = silentSamplesRemaining > 0 ? State::silentHold : State::idle;
        }

        [[nodiscard]] bool shouldProcess() const {
            return state == State::active;
        }

        [[nodiscard]] bool hasRecentSignal() const {
            return state != State::idle;
        }

        [[nodiscard]] float getPeakAbs() const {
            return lastPeakAbs;
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
            return static_cast<int>(sampleRate * silenceHoldMs * 0.001);
        }

        static constexpr float activateThresholdGain = 0.0000630957f;   // -84 dBFS
        static constexpr float deactivateThresholdGain = 0.0000316228f; // -90 dBFS
        static constexpr float silenceHoldMs = 220.0f;

        State state = State::idle;
        double sampleRate = 44100.0;
        float lastPeakAbs = 0.0f;
        int silentSamplesRemaining = 0;
    };
}
