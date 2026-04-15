#pragma once

#include <array>

#include <juce_audio_basics/juce_audio_basics.h>

#include "../dsp/sources/AnalysisSourceBuilder.h"
#include "../shared/SignalSlotConfiguration.h"

class SignalOutputMixer final {
public:
    struct SlotState {
        bool enabled = false;
        bool solo = false;
        Analyzer::SignalSource source = Analyzer::SignalSource::main;
        Analyzer::SignalMode mode = Analyzer::SignalMode::mid;
    };

    void prepare(int maximumBlockSize);
    void processBlock(juce::AudioBuffer<float> &mainBuffer,
                      const juce::AudioBuffer<float> *sidechainBuffer,
                      const std::array<SlotState, Shared::maxSignalSlots> &slots);

private:
    enum class ResolvedSignal {
        none,
        stereo,
        mid,
        side
    };

    struct SourceSelection {
        bool hasStereo = false;
        bool hasMid = false;
        bool hasSide = false;
    };

    static bool hasAnySolo(const std::array<SlotState, Shared::maxSignalSlots> &slots);
    static SourceSelection collectSelection(const std::array<SlotState, Shared::maxSignalSlots> &slots,
                                            Analyzer::SignalSource source);
    static ResolvedSignal resolveSelection(const SourceSelection &selection);

    void copyMainSnapshot(const juce::AudioBuffer<float> &mainBuffer);
    void accumulateResolvedSource(juce::AudioBuffer<float> &mainBuffer,
                                  const Analyzer::SourceSet &sourceSet,
                                  Analyzer::SignalSource source,
                                  ResolvedSignal resolvedSignal) const;
    void accumulateStereoViews(juce::AudioBuffer<float> &mainBuffer,
                               Analyzer::SignalView leftView,
                               Analyzer::SignalView rightView) const;

    Analyzer::AnalysisSourceBuilder sourceBuilder;
    juce::AudioBuffer<float> mainSnapshotBuffer;
};
