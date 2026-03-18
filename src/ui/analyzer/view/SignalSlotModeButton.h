#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../UiTheme.h"

class SignalSlotModeButton final : public juce::Component {
public:
    explicit SignalSlotModeButton(const Ui::Theme &themeToUse);

    void setLabel(const juce::String &text);

    std::function<void()> onPress;
    std::function<void()> onClick;

    void paint(juce::Graphics &g) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    const Ui::Theme &theme;
    juce::String label;
    bool hovered = false;
};
