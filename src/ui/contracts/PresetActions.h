#pragma once

#include <juce_core/juce_core.h>

#include "plugin/presets/PresetTypes.h"

class PresetActions {
public:
    virtual ~PresetActions() = default;

    [[nodiscard]] virtual PluginPresets::PresetActionResult loadPreset(const PluginPresets::PresetId& presetId) = 0;
    [[nodiscard]] virtual PluginPresets::PresetActionResult loadPreviousPreset() = 0;
    [[nodiscard]] virtual PluginPresets::PresetActionResult loadNextPreset() = 0;
    [[nodiscard]] virtual PluginPresets::PresetActionResult resetCurrentPreset() = 0;
    [[nodiscard]] virtual PluginPresets::PresetActionResult savePresetAs(const juce::String& name) = 0;
    [[nodiscard]] virtual PluginPresets::PresetActionResult overwritePreset(const PluginPresets::PresetId& presetId,
                                                                            const juce::String& name) = 0;
    [[nodiscard]] virtual PluginPresets::PresetActionResult deletePreset(const PluginPresets::PresetId& presetId) = 0;
    virtual void refreshPresetCatalog() = 0;
};
