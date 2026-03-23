#include "SignalSlotSourceToggle.h"

#include "../../UiRasterAssets.h"

SignalSlotSourceToggle::SignalSlotSourceToggle(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
}

void SignalSlotSourceToggle::setState(const Analyzer::SignalSource sourceToUse, const bool sidechainAvailableToUse) {
    source = sourceToUse;
    sidechainAvailable = sidechainAvailableToUse;
    rebuildCachedSwitch();
    repaint();
}

void SignalSlotSourceToggle::paint(juce::Graphics &g) {
    const auto mainActive = source == Analyzer::SignalSource::main;
    const auto sidechainActive = source == Analyzer::SignalSource::sidechain;

    const auto switchBounds = getSwitchBounds().getSmallestIntegerContainer();
    if (cachedSwitchImage.isValid())
        g.drawImageAt(cachedSwitchImage, switchBounds.getX(), switchBounds.getY());

    g.setFont(theme.metrics.slot.hintFontHeight + 1.0f);
    g.setColour(mainActive ? theme.controlText : theme.axisText);
    g.drawText("Main", getTopLabelBounds(), juce::Justification::centred);

    const auto sidechainColour = sidechainAvailable
                                     ? (sidechainActive ? theme.controlText : theme.axisText)
                                     : theme.subtleText.withMultipliedAlpha(0.7f);
    g.setColour(sidechainColour);
    g.drawText("Sidechain", getBottomLabelBounds(), juce::Justification::centred);
}

void SignalSlotSourceToggle::resized() {
    rebuildCachedSwitch();
}

void SignalSlotSourceToggle::mouseMove(const juce::MouseEvent &event) {
    hoveredHalf = getHoverHalf(event.position);
    const auto canClick = hoveredHalf == HoverHalf::main || (hoveredHalf == HoverHalf::sidechain && sidechainAvailable);
    setMouseCursor(canClick ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
}

void SignalSlotSourceToggle::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hoveredHalf = HoverHalf::none;
    setMouseCursor(juce::MouseCursor::NormalCursor);
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
    bounds.setHeight(bounds.getCentreY());
    return bounds;
}

juce::Rectangle<float> SignalSlotSourceToggle::getSidechainBounds() const {
    auto bounds = getLocalBounds().toFloat();
    bounds.removeFromTop(bounds.getCentreY());
    return bounds;
}

juce::Rectangle<float> SignalSlotSourceToggle::getSwitchBounds() const {
    auto bounds = getLocalBounds().toFloat();
    const auto labelHeight = juce::jmin(11.0f, bounds.getHeight() * 0.32f);
    bounds.removeFromTop(labelHeight);
    bounds.removeFromBottom(labelHeight);

    const auto switchHeight = juce::jmax(8.0f, bounds.getHeight());
    const auto switchWidth = juce::jmax(4.0f, switchHeight * (44.0f / 102.0f));
    return juce::Rectangle<float>(switchWidth, switchHeight).withCentre(bounds.getCentre());
}

juce::Rectangle<int> SignalSlotSourceToggle::getTopLabelBounds() const {
    auto bounds = getLocalBounds();
    bounds.setHeight(static_cast<int>(std::round(getSwitchBounds().getY())));
    return bounds;
}

juce::Rectangle<int> SignalSlotSourceToggle::getBottomLabelBounds() const {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(static_cast<int>(std::round(getSwitchBounds().getBottom())));
    return bounds;
}

SignalSlotSourceToggle::HoverHalf SignalSlotSourceToggle::getHoverHalf(const juce::Point<float> position) const {
    if (getMainBounds().contains(position))
        return HoverHalf::main;
    if (getSidechainBounds().contains(position))
        return HoverHalf::sidechain;
    return HoverHalf::none;
}

void SignalSlotSourceToggle::rebuildCachedSwitch() {
    const auto switchBounds = getSwitchBounds().getSmallestIntegerContainer();
    if (switchBounds.isEmpty()) {
        cachedSwitchImage = {};
        return;
    }

    const auto &sourceImage = Ui::getRasterAsset(source == Analyzer::SignalSource::sidechain
                                                     ? Ui::RasterAssetId::switchDown
                                                     : Ui::RasterAssetId::switchUp);
    cachedSwitchImage = juce::Image(juce::Image::ARGB, switchBounds.getWidth(), switchBounds.getHeight(), true);
    juce::Graphics graphics(cachedSwitchImage);
    graphics.drawImage(sourceImage,
                       0,
                       0,
                       switchBounds.getWidth(),
                       switchBounds.getHeight(),
                       0,
                       0,
                       sourceImage.getWidth(),
                       sourceImage.getHeight());
}
