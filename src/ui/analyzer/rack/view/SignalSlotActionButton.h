#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"

class SignalSlotActionButton final : public juce::Component,
                                     public juce::SettableTooltipClient {
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

    std::function<void()> onClick;

    void paint(juce::Graphics &g) override;
    void mouseEnter(const juce::MouseEvent &) override;
    void mouseExit(const juce::MouseEvent &) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    const Ui::Theme &theme;
    Style style;
    bool hovered = false;
};
