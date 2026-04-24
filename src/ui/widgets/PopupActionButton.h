#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"

class PopupActionButton final : public juce::Button {
public:
    enum class Style {
        secondary,
        primary
    };

    explicit PopupActionButton(const Ui::Theme& themeToUse);

    void setLabel(juce::String newLabel);
    void setStyle(Style newStyle);

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

private:
    const Ui::Theme& theme;
    juce::String label;
    Style style = Style::secondary;
};
