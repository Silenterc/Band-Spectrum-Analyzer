#include "RasterRectanglePadButton.h"

#include "ui/theme/UiIcons.h"
#include "ui/theme/UiRasterAssets.h"

RasterRectanglePadButton::RasterRectanglePadButton(const Ui::Theme& themeToUse,
                                                   juce::String labelText)
    : juce::Button({}),
      theme(themeToUse),
      label(std::move(labelText)) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTriggeredOnMouseDown(false);
}

juce::Rectangle<int> RasterRectanglePadButton::getPreferredBounds() const {
    return {0, 0, theme.metrics.rectanglePad.width, theme.metrics.rectanglePad.height};
}

juce::Point<float> RasterRectanglePadButton::getVisualCenterOffset() const {
    return {theme.metrics.rectanglePad.visualCenterOffsetX,
            theme.metrics.rectanglePad.visualCenterOffsetY};
}

bool RasterRectanglePadButton::isActive() const {
    return active;
}

bool RasterRectanglePadButton::hitTest(const int x, const int y) {
    return padBounds.contains(x, y);
}

void RasterRectanglePadButton::paintButton(juce::Graphics& g,
                                           const bool isMouseOverButton,
                                           const bool isButtonDown) {
    juce::ignoreUnused(isMouseOverButton, isButtonDown);

    const auto& padImage = active ? cachedOnImage : cachedOffImage;
    if (padImage.isValid())
        g.drawImageAt(padImage, padBounds.getX(), padBounds.getY());

    const auto markingColour = active
                                   ? activeMarkingColourOverride.value_or(theme.hardwareMarkingDark)
                                   : theme.hardwareMarkingLight;

    if (icon.has_value())
        Ui::drawIcon(g, *icon, iconBounds.toFloat(), markingColour);

    if (label.isEmpty())
        return;

    g.setColour(markingColour);
    g.setFont(juce::FontOptions(theme.metrics.rectanglePad.labelFontHeight).withStyle("Bold"));
    g.drawText(label, padBounds, juce::Justification::centred, false);
}

void RasterRectanglePadButton::resized() {
    const auto& metrics = theme.metrics.rectanglePad;
    padBounds = getPreferredBounds().withCentre(getLocalBounds().getCentre());
    iconBounds = juce::Rectangle<int>(metrics.iconSide, metrics.iconSide).withCentre(padBounds.getCentre());
    rebuildCachedPadImages();
}

void RasterRectanglePadButton::setActive(const bool shouldBeActive) {
    if (active == shouldBeActive)
        return;

    active = shouldBeActive;
    repaint(padBounds);
}

void RasterRectanglePadButton::setLabel(juce::String newLabel) {
    if (label == newLabel)
        return;

    label = std::move(newLabel);
    repaint(padBounds);
}

void RasterRectanglePadButton::setIcon(std::optional<Ui::IconId> newIcon) {
    if (icon == newIcon)
        return;

    icon = newIcon;
    repaint(padBounds);
}

void RasterRectanglePadButton::setActiveMarkingColour(const juce::Colour newActiveMarkingColour) {
    if (activeMarkingColourOverride.has_value() && activeMarkingColourOverride.value() == newActiveMarkingColour)
        return;

    activeMarkingColourOverride = newActiveMarkingColour;
    repaint(padBounds);
}

void RasterRectanglePadButton::rebuildCachedPadImages() {
    if (padBounds.isEmpty()) {
        cachedOffImage = {};
        cachedOnImage = {};
        return;
    }

    cachedOffImage = Ui::getControlRasterAsset(Ui::ControlRasterAssetId::rectanglePadOff).rescaled(
        padBounds.getWidth(),
        padBounds.getHeight(),
        juce::Graphics::highResamplingQuality);
    cachedOnImage = Ui::getControlRasterAsset(Ui::ControlRasterAssetId::rectanglePadOn).rescaled(
        padBounds.getWidth(),
        padBounds.getHeight(),
        juce::Graphics::highResamplingQuality);
}
