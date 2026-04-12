#include "SignalSlotSwatchButton.h"

#include "shared/DefaultParameterValues.h"

SignalSlotSwatchButton::SignalSlotSwatchButton(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
}

void SignalSlotSwatchButton::setState(const int colourIndexToUse, const float opacityToUse) {
    colourIndex = colourIndexToUse;
    opacity = opacityToUse;
    repaint();
}

void SignalSlotSwatchButton::paint(juce::Graphics &g) {
    const auto bounds = getLocalBounds().toFloat();
    g.setColour(Ui::getSignalPresetColour(colourIndex).withAlpha(opacity));
    g.fillRoundedRectangle(bounds, theme.metrics.slot.swatchCornerRadius);
    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), theme.metrics.slot.swatchCornerRadius, 1.0f);
}

void SignalSlotSwatchButton::mouseDown(const juce::MouseEvent &event) {
    mouseDownPosition = event.position;
    dragStartOpacity = opacity;
    didOpacityDrag = false;

    if (onPress)
        onPress();
}

void SignalSlotSwatchButton::mouseDrag(const juce::MouseEvent &event) {
    const auto delta = event.position - mouseDownPosition;
    if (std::abs(delta.y) > 3.0f && std::abs(delta.y) >= std::abs(delta.x))
        didOpacityDrag = true;

    if (!didOpacityDrag || !onOpacityChanged)
        return;

    const auto newOpacity = juce::jlimit(Defaults::signalOpacityMin,
                                         Defaults::signalOpacityMax,
                                         dragStartOpacity
                                             + (mouseDownPosition.y - event.position.y) * theme.metrics.slot.opacityPixelsToValue);
    opacity = newOpacity;
    onOpacityChanged(newOpacity);
}

void SignalSlotSwatchButton::mouseUp(const juce::MouseEvent &) {
    if (!didOpacityDrag && onClick)
        onClick();
}

void SignalSlotSwatchButton::mouseDoubleClick(const juce::MouseEvent &) {
    if (onOpacityReset)
        onOpacityReset();
}
