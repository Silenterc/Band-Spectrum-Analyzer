#pragma once

#include <juce_core/juce_core.h>

#include "plugin/presets/PresetTypes.h"

namespace Ui::Presets {
    [[nodiscard]] juce::String makePresetActionMessage(const PluginPresets::PresetActionResult& result);
}
