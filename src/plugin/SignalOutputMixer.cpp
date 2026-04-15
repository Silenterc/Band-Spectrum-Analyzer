#include "SignalOutputMixer.h"

#include <algorithm>

namespace {
    std::pair<Analyzer::SignalView, Analyzer::SignalView> getStereoViews(const Analyzer::SourceSet &sourceSet,
                                                                         const Analyzer::SignalSource source) {
        if (source == Analyzer::SignalSource::main)
            return {sourceSet.mainLeft, sourceSet.mainRight};

        return {sourceSet.sidechainLeft, sourceSet.sidechainRight};
    }

    Analyzer::SignalView getMidView(const Analyzer::SourceSet &sourceSet,
                                    const Analyzer::SignalSource source) {
        return source == Analyzer::SignalSource::main
                   ? sourceSet.mainMid
                   : sourceSet.sidechainMid;
    }

    Analyzer::SignalView getSideView(const Analyzer::SourceSet &sourceSet,
                                     const Analyzer::SignalSource source) {
        return source == Analyzer::SignalSource::main
                   ? sourceSet.mainSide
                   : sourceSet.sidechainSide;
    }
}

void SignalOutputMixer::prepare(const int maximumBlockSize) {
    sourceBuilder.prepare(maximumBlockSize);
    mainSnapshotBuffer.setSize(2, maximumBlockSize, false, false, true);
}

void SignalOutputMixer::processBlock(juce::AudioBuffer<float> &mainBuffer,
                                     const juce::AudioBuffer<float> *sidechainBuffer,
                                     const std::array<SlotState, Shared::maxSignalSlots> &slots) {
    if (mainBuffer.getNumChannels() <= 0 || mainBuffer.getNumSamples() == 0)
        return;

    if (!hasAnySolo(slots))
        return;

    // Need to copy since the left/right points straight to the buffer
    copyMainSnapshot(mainBuffer);
    const auto sourceSet = sourceBuilder.build(mainSnapshotBuffer, sidechainBuffer);

    mainBuffer.clear();
    accumulateResolvedSource(mainBuffer,
                             sourceSet,
                             Analyzer::SignalSource::main,
                             resolveSelection(collectSelection(slots, Analyzer::SignalSource::main)));
    accumulateResolvedSource(mainBuffer,
                             sourceSet,
                             Analyzer::SignalSource::sidechain,
                             resolveSelection(collectSelection(slots, Analyzer::SignalSource::sidechain)));
}

bool SignalOutputMixer::hasAnySolo(const std::array<SlotState, Shared::maxSignalSlots> &slots) {
    return std::any_of(slots.begin(), slots.end(), [](const SlotState &slot) {
        return slot.enabled && slot.solo;
    });
}

SignalOutputMixer::SourceSelection SignalOutputMixer::collectSelection(
    const std::array<SlotState, Shared::maxSignalSlots> &slots,
    const Analyzer::SignalSource source) {
    SourceSelection selection;

    for (const auto &slot: slots) {
        if (!slot.enabled || !slot.solo || slot.source != source)
            continue;

        switch (slot.mode) {
            case Analyzer::SignalMode::mid:
                selection.hasMid = true;
                break;
            case Analyzer::SignalMode::side:
                selection.hasSide = true;
                break;
            case Analyzer::SignalMode::stereo:
                selection.hasStereo = true;
                break;
        }
    }

    return selection;
}

SignalOutputMixer::ResolvedSignal SignalOutputMixer::resolveSelection(const SourceSelection &selection) {
    if (selection.hasStereo || (selection.hasMid && selection.hasSide))
        return ResolvedSignal::stereo;

    if (selection.hasMid)
        return ResolvedSignal::mid;

    if (selection.hasSide)
        return ResolvedSignal::side;

    return ResolvedSignal::none;
}

void SignalOutputMixer::copyMainSnapshot(const juce::AudioBuffer<float> &mainBuffer) {
    const auto numChannels = juce::jmax(1, mainBuffer.getNumChannels());
    const auto numSamples = mainBuffer.getNumSamples();
    mainSnapshotBuffer.setSize(numChannels, numSamples, false, false, true);

    for (int channelIndex = 0; channelIndex < numChannels; ++channelIndex)
        mainSnapshotBuffer.copyFrom(channelIndex, 0, mainBuffer, channelIndex, 0, numSamples);
}

void SignalOutputMixer::accumulateResolvedSource(juce::AudioBuffer<float> &mainBuffer,
                                                 const Analyzer::SourceSet &sourceSet,
                                                 const Analyzer::SignalSource source,
                                                 const ResolvedSignal resolvedSignal) const {
    switch (resolvedSignal) {
        case ResolvedSignal::none:
            return;
        case ResolvedSignal::stereo: {
            const auto [leftView, rightView] = getStereoViews(sourceSet, source);
            accumulateStereoViews(mainBuffer, leftView, rightView);
            return;
        }
        case ResolvedSignal::mid: {
            const auto midView = getMidView(sourceSet, source);
            accumulateStereoViews(mainBuffer, midView, midView);
            return;
        }
        case ResolvedSignal::side: {
            const auto sideView = getSideView(sourceSet, source);
            if (sideView.data == nullptr || sideView.numSamples == 0)
                return;

            auto *leftOutput = mainBuffer.getWritePointer(0);
            auto *rightOutput = mainBuffer.getNumChannels() > 1 ? mainBuffer.getWritePointer(1) : nullptr;
            const auto numSamples = std::min(sideView.numSamples, static_cast<size_t>(mainBuffer.getNumSamples()));

            for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                const auto sideSample = sideView.data[sampleIndex];
                if (rightOutput != nullptr) {
                    leftOutput[sampleIndex] += sideSample;
                    rightOutput[sampleIndex] -= sideSample;
                }
            }

            return;
        }
    }
}

void SignalOutputMixer::accumulateStereoViews(juce::AudioBuffer<float> &mainBuffer,
                                              const Analyzer::SignalView leftView,
                                              const Analyzer::SignalView rightView) const {
    if (leftView.data == nullptr || rightView.data == nullptr)
        return;

    auto *leftOutput = mainBuffer.getWritePointer(0);
    auto *rightOutput = mainBuffer.getNumChannels() > 1 ? mainBuffer.getWritePointer(1) : nullptr;
    const auto numSamples = std::min({leftView.numSamples,
                                      rightView.numSamples,
                                      static_cast<size_t>(mainBuffer.getNumSamples())});

    for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
        const auto leftSample = leftView.data[sampleIndex];
        const auto rightSample = rightView.data[sampleIndex];
        if (rightOutput != nullptr) {
            leftOutput[sampleIndex] += leftSample;
            rightOutput[sampleIndex] += rightSample;
        } else {
            leftOutput[sampleIndex] += 0.5f * (leftSample + rightSample);
        }
    }
}
