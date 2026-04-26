#pragma once

#include <juce_core/juce_core.h>

#include "ui/presets/state/PresetUiState.h"

class PresetActions {
public:
    virtual ~PresetActions() = default;

    [[nodiscard]] virtual Ui::Presets::PresetActionResult loadPreset(const Ui::Presets::PresetId& presetId) = 0;
    [[nodiscard]] virtual Ui::Presets::PresetActionResult loadPreviousPreset() = 0;
    [[nodiscard]] virtual Ui::Presets::PresetActionResult loadNextPreset() = 0;
    [[nodiscard]] virtual Ui::Presets::PresetActionResult resetCurrentPreset() = 0;
    [[nodiscard]] virtual Ui::Presets::PresetActionResult savePresetAs(const juce::String& name) = 0;
    [[nodiscard]] virtual Ui::Presets::PresetActionResult overwritePreset(const Ui::Presets::PresetId& presetId,
                                                                          const juce::String& name) = 0;
    [[nodiscard]] virtual Ui::Presets::PresetActionResult deletePreset(const Ui::Presets::PresetId& presetId) = 0;
    virtual void refreshPresetCatalog() = 0;
};
