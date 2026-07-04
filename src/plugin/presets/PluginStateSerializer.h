#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PresetTypes.h"
#include "plugin/state/SignalSlotOrderState.h"

class PluginStateSerializer final {
public:
    // Non-const parameters because juce::AudioProcessorValueTreeState::copyState() locks internally.
    [[nodiscard]] PluginPresets::PluginStateSnapshot captureState(
        juce::AudioProcessorValueTreeState& parameters,
        const SignalSlotOrderState& signalSlotOrderState) const;

    bool applyState(const PluginPresets::PluginStateSnapshot& snapshot,
                    juce::AudioProcessorValueTreeState& parameters,
                    SignalSlotOrderState& signalSlotOrderState) const;

    [[nodiscard]] bool statesEqual(const PluginPresets::PluginStateSnapshot& lhs,
                                   const PluginPresets::PluginStateSnapshot& rhs) const;

private:
    [[nodiscard]] juce::ValueTree normaliseSnapshotState(juce::ValueTree state) const;
    [[nodiscard]] juce::ValueTree mergeSnapshotIntoCurrentState(const juce::ValueTree& snapshotState,
                                                                const juce::ValueTree& currentState) const;
};
