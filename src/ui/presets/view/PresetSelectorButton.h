#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"

class PresetSelectorButton final : public juce::Button {
public:
    explicit PresetSelectorButton(const Ui::Theme& themeToUse);

    void setDisplayText(juce::String newDisplayText);

    std::function<void()> onPressed;
    std::function<void()> onDownArrow;

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    void resized() override;
    void buttonStateChanged() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    const Ui::Theme& theme;
    juce::String displayText;
    juce::Image cachedBackgroundImage;
    bool wasDown = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSelectorButton)
};
