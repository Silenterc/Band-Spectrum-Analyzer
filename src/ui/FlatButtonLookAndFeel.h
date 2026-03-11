#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "UiTheme.h"

class FlatButtonLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    explicit FlatButtonLookAndFeel(const Ui::Theme &themeToUse)
        : theme(themeToUse) {
    }

    void drawButtonBackground(juce::Graphics &g,
                              juce::Button &button,
                              const juce::Colour &backgroundColour,
                              bool isMouseOverButton,
                              bool isButtonDown) override {
        auto bounds = button.getLocalBounds().toFloat();
        auto fill = backgroundColour;

        if (isButtonDown) {
            fill = fill.darker(0.08f);
        } else if (isMouseOverButton) {
            fill = backgroundColour == theme.controlSurface
                       ? theme.controlSurfaceHover
                       : backgroundColour.brighter(0.08f);
        }

        g.setColour(fill);
        g.fillRoundedRectangle(bounds, theme.metrics.slot.cellCornerRadius);
    }

private:
    const Ui::Theme &theme;
};
