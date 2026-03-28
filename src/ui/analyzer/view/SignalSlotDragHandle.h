#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../UiTheme.h"

class SignalSlotDragHandle final : public juce::Component {
public:
    explicit SignalSlotDragHandle(const Ui::Theme &themeToUse);

    void setDragged(bool isDraggedValue);

    std::function<void(float)> onDragStarted;
    std::function<void(float)> onDragged;
    std::function<void(float)> onDragEnded;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    float getParentRelativeX(const juce::MouseEvent &event) const;
    void rebuildCachedImages();

    const Ui::Theme &theme;
    juce::Point<float> mouseDownPosition;
    bool hovered = false;
    bool dragged = false;
    bool trackingDrag = false;
    juce::Rectangle<int> padBounds;
    juce::Rectangle<int> iconBounds;
    juce::Image cachedOffImage;
    juce::Image cachedOnImage;
};
