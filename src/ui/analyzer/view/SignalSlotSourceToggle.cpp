#include "SignalSlotSourceToggle.h"

SignalSlotSourceToggle::SignalSlotSourceToggle(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
}

void SignalSlotSourceToggle::setState(const Analyzer::SignalSource sourceToUse, const bool sidechainAvailableToUse) {
    source = sourceToUse;
    sidechainAvailable = sidechainAvailableToUse;
    repaint();
}

void SignalSlotSourceToggle::paint(juce::Graphics &g) {
    const auto bounds = getLocalBounds().toFloat();
    const auto radius = theme.metrics.slot.buttonCornerRadius;
    const auto paintHalf = [this, &g, radius](const juce::Rectangle<float> &halfBounds,
                                              const bool active,
                                              const bool hovered,
                                              const bool enabled,
                                              const juce::String &label,
                                              const bool topHalf) {
        auto fill = active ? theme.accentButton : theme.controlSurface;
        if (hovered && enabled)
            fill = active ? theme.accentButton.brighter(0.14f) : theme.controlSurfaceHover;

        juce::Path path;
        path.addRoundedRectangle(halfBounds.getX(),
                                 halfBounds.getY(),
                                 halfBounds.getWidth(),
                                 halfBounds.getHeight(),
                                 radius,
                                 radius,
                                 topHalf,
                                 topHalf,
                                 !topHalf,
                                 !topHalf);
        g.setColour(enabled ? fill : fill.withMultipliedAlpha(0.45f));
        g.fillPath(path);
        g.setColour(enabled ? theme.controlText : theme.subtleText.withMultipliedAlpha(0.75f));
        g.setFont(theme.metrics.slot.hintFontHeight + 1.0f);
        g.drawText(label, halfBounds.toNearestInt(), juce::Justification::centred);
    };

    g.setColour(theme.controlSurface);
    g.fillRoundedRectangle(bounds, radius);
    paintHalf(getMainBounds(),
              source == Analyzer::SignalSource::main,
              hoveredHalf == HoverHalf::main,
              true,
              "Main",
              true);
    paintHalf(getSidechainBounds(),
              source == Analyzer::SignalSource::sidechain,
              hoveredHalf == HoverHalf::sidechain,
              sidechainAvailable,
              "Sidechain",
              false);
    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), radius, 1.0f);
    g.drawHorizontalLine(static_cast<int>(std::round(getMainBounds().getBottom())),
                         bounds.getX() + 1.0f,
                         bounds.getRight() - 1.0f);
}

void SignalSlotSourceToggle::mouseMove(const juce::MouseEvent &event) {
    hoveredHalf = getHoverHalf(event.position);
    const auto canClick = hoveredHalf == HoverHalf::main || (hoveredHalf == HoverHalf::sidechain && sidechainAvailable);
    setMouseCursor(canClick ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
    repaint();
}

void SignalSlotSourceToggle::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hoveredHalf = HoverHalf::none;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    repaint();
}

void SignalSlotSourceToggle::mouseUp(const juce::MouseEvent &event) {
    if (event.mouseWasDraggedSinceMouseDown())
        return;

    const auto half = getHoverHalf(event.position);
    if (half == HoverHalf::main && onSourceSelected)
        onSourceSelected(Analyzer::SignalSource::main);
    else if (half == HoverHalf::sidechain && sidechainAvailable && onSourceSelected)
        onSourceSelected(Analyzer::SignalSource::sidechain);
}

juce::Rectangle<float> SignalSlotSourceToggle::getMainBounds() const {
    auto bounds = getLocalBounds().toFloat();
    bounds.setHeight(bounds.getHeight() * 0.5f);
    return bounds;
}

juce::Rectangle<float> SignalSlotSourceToggle::getSidechainBounds() const {
    auto bounds = getLocalBounds().toFloat();
    bounds.removeFromTop(bounds.getHeight() * 0.5f);
    return bounds;
}

SignalSlotSourceToggle::HoverHalf SignalSlotSourceToggle::getHoverHalf(const juce::Point<float> position) const {
    if (getMainBounds().contains(position))
        return HoverHalf::main;
    if (getSidechainBounds().contains(position))
        return HoverHalf::sidechain;
    return HoverHalf::none;
}
