#pragma once

#include <optional>

#include <juce_data_structures/juce_data_structures.h>

#include "PresetConstants.h"

namespace PluginPresets {
    enum class PresetOrigin {
        factory,
        user
    };

    struct PluginStateSnapshot {
        juce::ValueTree state;

        [[nodiscard]] bool isValid() const;
    };

    struct PresetDocument {
        int formatVersion = Constants::currentDocumentFormatVersion;
        juce::String id;
        juce::String name;
        PresetOrigin origin = PresetOrigin::factory;
        juce::String createdAtUtc;
        juce::String updatedAtUtc;
        PluginStateSnapshot pluginState;
    };

    [[nodiscard]] juce::String toString(PresetOrigin origin);
    [[nodiscard]] std::optional<PresetOrigin> presetOriginFromString(const juce::String& text);

    bool operator==(const PluginStateSnapshot& lhs, const PluginStateSnapshot& rhs);

    inline bool operator!=(const PluginStateSnapshot& lhs, const PluginStateSnapshot& rhs) {
        return !(lhs == rhs);
    }
}
