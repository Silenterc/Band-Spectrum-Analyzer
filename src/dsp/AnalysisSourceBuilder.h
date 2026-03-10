#pragma once

#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>

namespace Analyzer {
    /**
     * Lightweight read-only signal view: data pointer + sample count
     * Points either into the host-owned input buffer or into an engine-owned derived buffer.
     */
    struct SignalView {
        const float *data = nullptr;
        size_t numSamples = 0;

        SignalView() = default;

        SignalView(const float *dataToUse, size_t numSamplesToUse)
            : data(dataToUse), numSamples(numSamplesToUse) {
        }
    };

    /**
     * Reusable source views for the current block
     * Left/right views point directly into the host-owned audio block.
     * Mono-sum views point into engine-owned temporary buffers computed for this block.
     */
    struct SourceSet {
        SignalView mainLeft{};
        SignalView mainRight{};
        SignalView mainMid{};
        SignalView mainSide{};
        SignalView sidechainLeft{};
        SignalView sidechainRight{};
        SignalView sidechainMid{};
        SignalView sidechainSide{};
    };

    /**
     * Builds block-local analysis source views and owns derived temporary buffers.
     */
    class AnalysisSourceBuilder {
    public:
        void prepare(int maximumBlockSize);

        SourceSet build(const juce::AudioBuffer<float> &mainBuffer,
                        const juce::AudioBuffer<float> *sidechainBuffer);

    private:
        void updateMidAndSideBuffers(const juce::AudioBuffer<float> &buffer,
                                     std::vector<float> &midBuffer,
                                     std::vector<float> &sideBuffer);

        // Engine-owned storage for derived mid samples from the main input
        std::vector<float> mainMidBuffer;
        // Engine-owned storage for derived side samples from the main input
        std::vector<float> mainSideBuffer;
        // Engine-owned storage for derived mid samples from the sidechain input
        std::vector<float> sidechainMidBuffer;
        // Engine-owned storage for derived side samples from the sidechain input
        std::vector<float> sidechainSideBuffer;
    };
}
