#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../UiButtonDrawing.h"

class AnalyzerFreezeButton final : public juce::Button {
public:
    explicit AnalyzerFreezeButton(const Ui::Theme &themeToUse)
        : juce::Button({}),
          theme(themeToUse) {
    }

    void setFrozen(const bool isFrozenValue) {
        if (isFrozen == isFrozenValue)
            return;

        isFrozen = isFrozenValue;
        repaint();
    }

    void paintButton(juce::Graphics &g, const bool isMouseOverButton, const bool isButtonDown) override {
        auto style = Ui::getSnowflakeButtonStyle(theme, isFrozen, isMouseOverButton);
        if (isButtonDown) {
            style.fill = style.fill.darker(0.08f);
        }

        Ui::drawSnowflakeActionButton(g,
                                      getLocalBounds().toFloat(),
                                      theme,
                                      style,
                                      2.5f);
    }

private:
    const Ui::Theme &theme;
    bool isFrozen = false;
};
