#pragma once

#include <optional>
#include <utility>
#include <vector>

#include <juce_data_structures/juce_data_structures.h>

namespace Ui::Presets {
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

    struct PresetDescriptor {
        PresetId id;
        juce::String name;
        PresetOrigin origin = PresetOrigin::factory;
        bool isDeletable = false;
        bool shadowsFactoryPreset = false;
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

        [[nodiscard]] static PresetActionResult ok() {
            return {};
        }

        [[nodiscard]] static PresetActionResult failed(const PresetActionErrorCode code,
                                                       juce::String presetName = {}) {
            PresetActionResult result;
            result.succeeded = false;
            result.error = PresetActionError{code, std::move(presetName)};
            return result;
        }

        [[nodiscard]] explicit operator bool() const noexcept {
            return succeeded;
        }
    };

    inline bool operator==(const PresetDescriptor& lhs, const PresetDescriptor& rhs) {
        return lhs.id == rhs.id
               && lhs.name == rhs.name
               && lhs.origin == rhs.origin
               && lhs.isDeletable == rhs.isDeletable
               && lhs.shadowsFactoryPreset == rhs.shadowsFactoryPreset;
    }

    inline bool operator==(const PresetUiSnapshot& lhs, const PresetUiSnapshot& rhs) {
        return lhs.presets == rhs.presets
               && lhs.selectedPresetId == rhs.selectedPresetId
               && lhs.selectedPresetName == rhs.selectedPresetName
               && lhs.selectionStatus == rhs.selectionStatus
               && lhs.isSelectedPresetUser == rhs.isSelectedPresetUser
               && lhs.canLoadPrevious == rhs.canLoadPrevious
               && lhs.canLoadNext == rhs.canLoadNext
               && lhs.canReset == rhs.canReset
               && lhs.canSave == rhs.canSave;
    }

    inline bool operator==(const PresetActionError& lhs, const PresetActionError& rhs) {
        return lhs.code == rhs.code
               && lhs.presetName == rhs.presetName;
    }

    inline bool operator==(const PresetActionResult& lhs, const PresetActionResult& rhs) {
        return lhs.succeeded == rhs.succeeded
               && lhs.error == rhs.error;
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
