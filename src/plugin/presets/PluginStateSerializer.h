#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PresetTypes.h"
#include "plugin/state/SignalSlotOrderState.h"

class PluginStateSerializer final {
public:
    [[nodiscard]] PluginPresets::PluginStateSnapshot captureState(
        const juce::AudioProcessorValueTreeState& parameters,
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
