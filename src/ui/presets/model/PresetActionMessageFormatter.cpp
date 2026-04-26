#include "PresetActionMessageFormatter.h"

namespace Ui::Presets {
    juce::String makePresetActionMessage(const Ui::Presets::PresetActionResult& result) {
        if (result.succeeded || !result.error.has_value())
            return {};

        const auto& error = *result.error;
        switch (error.code) {
            case Ui::Presets::PresetActionErrorCode::emptyName:
                return "Enter a preset name.";
            case Ui::Presets::PresetActionErrorCode::duplicateUserPresetName:
                return error.presetName.isNotEmpty()
                           ? "\"" + error.presetName + "\" already exists."
                           : juce::String("A preset with that name already exists.");
            case Ui::Presets::PresetActionErrorCode::presetNotFound:
                return error.presetName.isNotEmpty()
                           ? "Couldn't find \"" + error.presetName + "\"."
                           : juce::String("That preset is no longer available.");
            case Ui::Presets::PresetActionErrorCode::presetNotLoadable:
                return error.presetName.isNotEmpty()
                           ? "Couldn't load \"" + error.presetName + "\"."
                           : juce::String("Couldn't load that preset.");
            case Ui::Presets::PresetActionErrorCode::saveFailed:
                return error.presetName.isNotEmpty()
                           ? "Couldn't save \"" + error.presetName + "\"."
                           : juce::String("Couldn't save the preset.");
            case Ui::Presets::PresetActionErrorCode::overwriteFailed:
                return error.presetName.isNotEmpty()
                           ? "Couldn't overwrite \"" + error.presetName + "\"."
                           : juce::String("Couldn't overwrite the preset.");
            case Ui::Presets::PresetActionErrorCode::deleteFailed:
                return error.presetName.isNotEmpty()
                           ? "Couldn't delete \"" + error.presetName + "\"."
                           : juce::String("Couldn't delete the preset.");
            case Ui::Presets::PresetActionErrorCode::invalidOverwriteTarget:
                return error.presetName.isNotEmpty()
                           ? "Use Save As to rename \"" + error.presetName + "\"."
                           : juce::String("That preset can't be overwritten.");
        }

        jassertfalse;
        return "Preset action failed.";
    }
}
