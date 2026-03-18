#include "SignalSlotModeButton.h"

SignalSlotModeButton::SignalSlotModeButton(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void SignalSlotModeButton::setLabel(const juce::String &text) {
    label = text;
    repaint();
}

void SignalSlotModeButton::paint(juce::Graphics &g) {
    const auto bounds = getLocalBounds().toFloat();
    g.setColour(hovered ? theme.controlSurfaceHover : theme.controlSurface);
    g.fillRoundedRectangle(bounds, theme.metrics.slot.buttonCornerRadius);
    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), theme.metrics.slot.buttonCornerRadius, 1.0f);
    g.setColour(theme.controlText);
    g.setFont(theme.metrics.slot.titleFontHeight);
    g.drawText(label, getLocalBounds(), juce::Justification::centred);
}

void SignalSlotModeButton::mouseDown(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    if (onPress)
        onPress();
}

void SignalSlotModeButton::mouseEnter(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hovered = true;
    repaint();
}

void SignalSlotModeButton::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hovered = false;
    repaint();
}

void SignalSlotModeButton::mouseUp(const juce::MouseEvent &event) {
    if (!event.mouseWasDraggedSinceMouseDown() && onClick)
        onClick();
}
