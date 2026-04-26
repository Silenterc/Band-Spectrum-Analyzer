#pragma once

#include <juce_core/juce_core.h>

#include "ui/presets/state/PresetUiState.h"

namespace Ui::Presets {
    [[nodiscard]] juce::String makePresetActionMessage(const Ui::Presets::PresetActionResult& result);
}
