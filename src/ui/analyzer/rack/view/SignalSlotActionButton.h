#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"

class SignalSlotActionButton final : public juce::Button {
public:
    enum class Content {
        text,
        cancel,
        power,
        snowflake
    };

    struct Style {
        Content content = Content::text;
        juce::String text;
        float fontHeight = 11.0f;
        juce::Colour fill;
        juce::Colour hoverFill;
        juce::Colour foreground;
        bool drawsBackground = true;
    };

    explicit SignalSlotActionButton(const Ui::Theme &themeToUse);

    void setStyle(const Style &styleToUse);

    void paintButton(juce::Graphics &g, bool isMouseOverButton, bool isButtonDown) override;

private:
    const Ui::Theme &theme;
    Style style;
};
