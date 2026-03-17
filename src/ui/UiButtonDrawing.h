#pragma once

#include <juce_graphics/juce_graphics.h>

#include "UiIcons.h"
#include "UiTheme.h"

namespace Ui {
    struct SnowflakeButtonStyle {
        juce::Colour fill;
        juce::Colour icon;
    };

    inline SnowflakeButtonStyle getSnowflakeButtonStyle(const Theme &theme,
                                                        const bool isFrozen,
                                                        const bool isHovered) {
        SnowflakeButtonStyle style;
        style.fill = isFrozen
                         ? (isHovered ? theme.accentButton.brighter(0.14f) : theme.accentButton)
                         : (isHovered ? theme.controlSurfaceHover : theme.controlSurface);
        style.icon = isFrozen ? theme.controlText : theme.subtleText;
        return style;
    }

    inline void drawSnowflakeActionButton(juce::Graphics &g,
                                          const juce::Rectangle<float> &bounds,
                                          const Theme &theme,
                                          const SnowflakeButtonStyle &style,
                                          const float iconInset) {
        g.setColour(style.fill);
        g.fillRoundedRectangle(bounds, theme.metrics.slot.buttonCornerRadius);
        Ui::drawSnowflakeIcon(g, bounds.reduced(iconInset), style.icon);
    }
}
