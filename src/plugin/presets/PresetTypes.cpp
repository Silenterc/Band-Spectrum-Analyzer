#include "PresetTypes.h"

namespace PluginPresets {
    bool PluginStateSnapshot::isValid() const {
        return state.isValid();
    }

    juce::String toString(const PresetOrigin origin) {
        switch (origin) {
            case PresetOrigin::factory:
                return "factory";
            case PresetOrigin::user:
                return "user";
        }

        jassertfalse;
        return "factory";
    }

    std::optional<PresetOrigin> presetOriginFromString(const juce::String& text) {
        if (text.equalsIgnoreCase("factory"))
            return PresetOrigin::factory;

        if (text.equalsIgnoreCase("user"))
            return PresetOrigin::user;

        return std::nullopt;
    }

    bool operator==(const PluginStateSnapshot& lhs, const PluginStateSnapshot& rhs) {
        if (!lhs.state.isValid() || !rhs.state.isValid())
            return lhs.state.isValid() == rhs.state.isValid();

        return lhs.state.isEquivalentTo(rhs.state);
    }

    bool operator==(const PresetDescriptor& lhs, const PresetDescriptor& rhs) {
        return lhs.id == rhs.id
               && lhs.name == rhs.name
               && lhs.origin == rhs.origin
               && lhs.isDeletable == rhs.isDeletable
               && lhs.shadowsFactoryPreset == rhs.shadowsFactoryPreset;
    }

    bool operator==(const PresetUiSnapshot& lhs, const PresetUiSnapshot& rhs) {
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

    PresetActionResult PresetActionResult::ok() {
        return {};
    }

    PresetActionResult PresetActionResult::failed(const PresetActionErrorCode code,
                                                  juce::String presetName) {
        PresetActionResult result;
        result.succeeded = false;
        result.error = PresetActionError{code, std::move(presetName)};
        return result;
    }

    bool operator==(const PresetActionError& lhs, const PresetActionError& rhs) {
        return lhs.code == rhs.code
               && lhs.presetName == rhs.presetName;
    }

    bool operator==(const PresetActionResult& lhs, const PresetActionResult& rhs) {
        return lhs.succeeded == rhs.succeeded
               && lhs.error == rhs.error;
    }
}
