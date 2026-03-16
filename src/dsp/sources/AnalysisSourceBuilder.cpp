#include "AnalysisSourceBuilder.h"

namespace Analyzer {
    void AnalysisSourceBuilder::prepare(int maximumBlockSize) {
        mainMidBuffer.resize(static_cast<size_t>(maximumBlockSize));
        mainSideBuffer.resize(static_cast<size_t>(maximumBlockSize));
        sidechainMidBuffer.resize(static_cast<size_t>(maximumBlockSize));
        sidechainSideBuffer.resize(static_cast<size_t>(maximumBlockSize));
    }

    SourceSet AnalysisSourceBuilder::build(const juce::AudioBuffer<float> &mainBuffer,
                                           const juce::AudioBuffer<float> *sidechainBuffer) {
        SourceSet sourceSet;

        const auto mainNumSamples = static_cast<size_t>(mainBuffer.getNumSamples());
        const auto *mainLeft = mainBuffer.getReadPointer(0);
        const auto *mainRight = mainBuffer.getNumChannels() > 1 ? mainBuffer.getReadPointer(1) : mainLeft;

        sourceSet.mainLeft = SignalView(mainLeft, mainNumSamples);
        sourceSet.mainRight = SignalView(mainRight, mainNumSamples);

        updateMidAndSideBuffers(mainBuffer, mainMidBuffer, mainSideBuffer);
        sourceSet.mainMid = SignalView(mainMidBuffer.data(), mainMidBuffer.size());
        sourceSet.mainSide = SignalView(mainSideBuffer.data(), mainSideBuffer.size());

        if (sidechainBuffer == nullptr || sidechainBuffer->getNumChannels() <= 0)
            return sourceSet;

        const auto sidechainNumSamples = static_cast<size_t>(sidechainBuffer->getNumSamples());
        const auto *sidechainLeft = sidechainBuffer->getReadPointer(0);
        const auto *sidechainRight = sidechainBuffer->getNumChannels() > 1
                                         ? sidechainBuffer->getReadPointer(1)
                                         : sidechainLeft;

        sourceSet.sidechainLeft = SignalView(sidechainLeft, sidechainNumSamples);
        sourceSet.sidechainRight = SignalView(sidechainRight, sidechainNumSamples);

        updateMidAndSideBuffers(*sidechainBuffer, sidechainMidBuffer, sidechainSideBuffer);
        sourceSet.sidechainMid = SignalView(sidechainMidBuffer.data(),
                                            sidechainMidBuffer.size());
        sourceSet.sidechainSide = SignalView(sidechainSideBuffer.data(),
                                             sidechainSideBuffer.size());

        return sourceSet;
    }

    void AnalysisSourceBuilder::updateMidAndSideBuffers(const juce::AudioBuffer<float> &buffer,
                                                        std::vector<float> &midBuffer,
                                                        std::vector<float> &sideBuffer) {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = static_cast<size_t>(buffer.getNumSamples());
        const auto *leftChannel = buffer.getReadPointer(0);
        const auto *rightChannel = numChannels > 1 ? buffer.getReadPointer(1) : nullptr;
        midBuffer.resize(numSamples);
        sideBuffer.resize(numSamples);

        // Precompute mid and side once so processors can reuse them
        for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
            const auto leftSample = leftChannel[sampleIndex];
            const auto rightSample = rightChannel != nullptr ? rightChannel[sampleIndex] : leftSample;
            midBuffer[sampleIndex] = 0.5f * (leftSample + rightSample);
            sideBuffer[sampleIndex] = 0.5f * (leftSample - rightSample);
        }
    }
}
