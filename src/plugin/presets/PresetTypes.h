#pragma once

#include <optional>
#include <vector>

#include <juce_data_structures/juce_data_structures.h>

namespace PluginPresets {
    using PresetId = juce::String;

    enum class PresetOrigin {
        factory,
        user
    };

    enum class PresetActionErrorCode {
        emptyName,
        duplicateUserPresetName,
        presetNotFound,
        presetNotLoadable,
        saveFailed,
        overwriteFailed,
        deleteFailed,
        invalidOverwriteTarget,
    };

    enum class PresetSelectionStatus {
        selectedClean,
        selectedDirty,
        unsaved
    };

    struct PluginStateSnapshot {
        juce::ValueTree state;

        [[nodiscard]] bool isValid() const;
    };

    struct PresetDescriptor {
        PresetId id;
        juce::String name;
        PresetOrigin origin = PresetOrigin::factory;
        bool isDeletable = false;
        bool shadowsFactoryPreset = false;
    };

    struct PresetDocument {
        int formatVersion = 1;
        PresetId id;
        juce::String name;
        PresetOrigin origin = PresetOrigin::factory;
        juce::String createdAtUtc;
        juce::String updatedAtUtc;
        PluginStateSnapshot pluginState;
    };

    struct PresetUiSnapshot {
        std::vector<PresetDescriptor> presets;
        std::optional<PresetId> selectedPresetId;
        juce::String selectedPresetName = "Default";
        PresetSelectionStatus selectionStatus = PresetSelectionStatus::unsaved;
        bool isSelectedPresetUser = false;
        bool canLoadPrevious = false;
        bool canLoadNext = false;
        bool canReset = false;
        bool canSave = true;
    };

    struct PresetActionError {
        PresetActionErrorCode code = PresetActionErrorCode::saveFailed;
        juce::String presetName;
    };

    struct PresetActionResult {
        bool succeeded = true;
        std::optional<PresetActionError> error;

        [[nodiscard]] static PresetActionResult ok();
        [[nodiscard]] static PresetActionResult failed(PresetActionErrorCode code,
                                                       juce::String presetName = {});
        [[nodiscard]] explicit operator bool() const noexcept {
            return succeeded;
        }
    };

    [[nodiscard]] juce::String toString(PresetOrigin origin);
    [[nodiscard]] std::optional<PresetOrigin> presetOriginFromString(const juce::String& text);

    bool operator==(const PluginStateSnapshot& lhs, const PluginStateSnapshot& rhs);
    bool operator==(const PresetDescriptor& lhs, const PresetDescriptor& rhs);
    bool operator==(const PresetUiSnapshot& lhs, const PresetUiSnapshot& rhs);
    bool operator==(const PresetActionError& lhs, const PresetActionError& rhs);
    bool operator==(const PresetActionResult& lhs, const PresetActionResult& rhs);

    inline bool operator!=(const PluginStateSnapshot& lhs, const PluginStateSnapshot& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator!=(const PresetDescriptor& lhs, const PresetDescriptor& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator!=(const PresetUiSnapshot& lhs, const PresetUiSnapshot& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator!=(const PresetActionError& lhs, const PresetActionError& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator!=(const PresetActionResult& lhs, const PresetActionResult& rhs) {
        return !(lhs == rhs);
    }
}
