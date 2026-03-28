#include "SignalSlotModeButton.h"

#include "../../UiRasterAssets.h"

SignalSlotModeButton::SignalSlotModeButton(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void SignalSlotModeButton::setLabel(const juce::String &text) {
    label = text;
    repaint();
}

void SignalSlotModeButton::paint(juce::Graphics &g) {
    const auto &slotMetrics = theme.metrics.slot;
    const auto bounds = getLocalBounds().toFloat();

    if (cachedBackground.isValid())
        g.drawImageAt(cachedBackground, 0, 0);

    if (hovered) {
        g.setColour(juce::Colours::white.withAlpha(slotMetrics.modeHoverAlpha));
        g.fillRoundedRectangle(bounds.reduced(1.5f, 1.5f), slotMetrics.modeHoverCornerRadius);
    }

    g.setColour(theme.hardwareMarkingDark);
    g.setFont(juce::FontOptions(slotMetrics.titleFontHeight + slotMetrics.modeTitleFontDelta, juce::Font::bold));
    g.drawText(label, getLocalBounds(), juce::Justification::centred);
}

void SignalSlotModeButton::resized() {
    rebuildCachedBackground();
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

void SignalSlotModeButton::rebuildCachedBackground() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        cachedBackground = {};
        return;
    }

    cachedBackground = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics graphics(cachedBackground);
    Ui::drawAssetWithin(graphics,
                        Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::screen),
                        bounds);
}
