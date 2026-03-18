#pragma once

#include <juce_graphics/juce_graphics.h>

#include "UiIcons.h"
#include "UiTheme.h"

namespace Ui {
    struct IconActionButtonStyle {
        juce::Colour fill;
        juce::Colour icon;
    };

    inline IconActionButtonStyle getIconActionButtonStyle(const Theme &theme,
                                                          const bool isActive,
                                                          const bool isHovered) {
        IconActionButtonStyle style;
        style.fill = isActive
                         ? (isHovered ? theme.accentButton.brighter(0.14f) : theme.accentButton)
                         : (isHovered ? theme.controlSurfaceHover : theme.controlSurface);
        style.icon = isActive ? theme.controlText : theme.subtleText;
        return style;
    }

    inline void drawSnowflakeActionButton(juce::Graphics &g,
                                          const juce::Rectangle<float> &bounds,
                                          const Theme &theme,
                                          const IconActionButtonStyle &style,
                                          const float iconInset) {
        g.setColour(style.fill);
        g.fillRoundedRectangle(bounds, theme.metrics.slot.buttonCornerRadius);
        Ui::drawSnowflakeIcon(g, bounds.reduced(iconInset), style.icon);
    }
}
