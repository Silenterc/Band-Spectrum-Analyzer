#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"

namespace Ui::Presets {
    [[nodiscard]] inline int getPresetPopupTitleBlockHeight(const Ui::Theme& theme) {
        const auto& metrics = theme.metrics.presetPopup;
        return metrics.titleTopGap
               + juce::roundToInt(metrics.titleFontHeight)
               + metrics.titleBottomGap;
    }

    [[nodiscard]] inline int getPresetPopupButtonRowWidth(const Ui::Theme& theme) {
        const auto& metrics = theme.metrics.presetPopup;
        return metrics.saveButtonWidth * 2 + metrics.saveButtonGap;
    }

    [[nodiscard]] inline int getPresetPopupStatusGap(const Ui::Theme& theme, const int statusHeight) {
        return statusHeight > 0 ? theme.metrics.presetPopup.statusTopGap : 0;
    }

    [[nodiscard]] inline juce::Rectangle<int> takePresetPopupButtonArea(juce::Rectangle<int>& bounds,
                                                                        const Ui::Theme& theme) {
        return bounds.removeFromTop(theme.metrics.presetPopup.saveButtonHeight);
    }

    [[nodiscard]] inline juce::Rectangle<int> makePresetPopupButtonRowBounds(const juce::Rectangle<int> buttonArea,
                                                                             const Ui::Theme& theme) {
        return juce::Rectangle<int>(getPresetPopupButtonRowWidth(theme),
                                    theme.metrics.presetPopup.saveButtonHeight)
            .withCentre(buttonArea.getCentre());
    }
}
