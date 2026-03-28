#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../UiTheme.h"

class SignalSlotSwatchButton final : public juce::Component {
public:
    explicit SignalSlotSwatchButton(const Ui::Theme &themeToUse);

    void setState(int colourIndexToUse, float opacityToUse);

    std::function<void()> onPress;
    std::function<void()> onClick;
    std::function<void(float)> onOpacityChanged;
    std::function<void()> onOpacityReset;

    void paint(juce::Graphics &g) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    void mouseDoubleClick(const juce::MouseEvent &event) override;

private:
    const Ui::Theme &theme;
    int colourIndex = 0;
    float opacity = Ui::defaultSignalOpacity;
    juce::Point<float> mouseDownPosition;
    float dragStartOpacity = Ui::defaultSignalOpacity;
    bool didOpacityDrag = false;
};
