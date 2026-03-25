#pragma once

#include <juce_graphics/juce_graphics.h>

#include "UiIcons.h"
#include "UiTheme.h"

namespace Ui {
    struct IconActionButtonStyle {
        juce::Colour fill;
        juce::Colour icon;
    };

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
