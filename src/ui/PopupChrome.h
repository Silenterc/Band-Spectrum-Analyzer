#pragma once

#include <juce_graphics/juce_graphics.h>

#include "UiTheme.h"

namespace Ui {
    inline void paintPopupShell(juce::Graphics &g, const juce::Rectangle<float> bounds, const Theme &theme) {
        const auto &popupMetrics = theme.metrics.popup;
        g.setColour(theme.controlSurface.withMultipliedBrightness(popupMetrics.shellBrightness));
        g.fillRoundedRectangle(bounds, popupMetrics.shellCornerRadius);
        g.setColour(theme.sectionDividerHighlight.withMultipliedAlpha(popupMetrics.shellBorderAlpha));
        g.drawRoundedRectangle(bounds.reduced(0.5f), popupMetrics.shellCornerRadius, 1.0f);
    }
}
