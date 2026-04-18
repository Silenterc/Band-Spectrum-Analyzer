#include "RasterIconButton.h"

#include "ui/theme/UiRasterAssets.h"

RasterIconButton::RasterIconButton(const Ui::Theme& themeToUse)
    : theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

int RasterIconButton::getPreferredSideLength() const {
    const auto& buttonImage = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::buttonOff);
    const auto logicalHeight = static_cast<float>(buttonImage.getHeight()) / theme.metrics.assets.rasterScale;
    return juce::jmax(1, juce::roundToInt(logicalHeight * juce::jmax(0.0f, scaleMultiplier)));
}

void RasterIconButton::setActive(const bool shouldBeActive) {
    if (active == shouldBeActive)
        return;

    active = shouldBeActive;
    repaint();
}

void RasterIconButton::setIcon(const Ui::IconId newIcon) {
    if (icon == newIcon)
        return;

    icon = newIcon;
    repaint(imageBounds);
}

void RasterIconButton::setScaleMultiplier(const float newScaleMultiplier) {
    const auto clampedScaleMultiplier = juce::jmax(0.0f, newScaleMultiplier);
    if (juce::approximatelyEqual(scaleMultiplier, clampedScaleMultiplier))
        return;

    scaleMultiplier = clampedScaleMultiplier;
    resized();
    repaint();
}

void RasterIconButton::setIconScaleMultiplier(const float newScaleMultiplier) {
    const auto clampedScaleMultiplier = juce::jmax(0.0f, newScaleMultiplier);
    if (juce::approximatelyEqual(iconScaleMultiplier, clampedScaleMultiplier))
        return;

    iconScaleMultiplier = clampedScaleMultiplier;
    resized();
    repaint();
}

void RasterIconButton::paint(juce::Graphics& g) {
    const auto& buttonImage = (active || pressed) ? cachedOnImage : cachedOffImage;
    if (buttonImage.isValid())
        g.drawImageAt(buttonImage, imageBounds.getX(), imageBounds.getY());

    const auto iconColour = (active || pressed) ? theme.hardwareMarkingDark : theme.hardwareMarkingLight;
    Ui::drawIcon(g, icon, iconBounds.toFloat(), iconColour);
}

void RasterIconButton::resized() {
    imageBounds = Ui::getScaledAssetBoundsWithin(Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::buttonOff),
                                                 theme.metrics.assets.rasterScale,
                                                 getLocalBounds(),
                                                 scaleMultiplier);
    iconBounds = Ui::getScaledInnerBounds(imageBounds, 0.28f, iconScaleMultiplier);
    rebuildCachedImages();
}

void RasterIconButton::mouseDown(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);

    if (pressed)
        return;

    pressed = true;
    repaint();
}

void RasterIconButton::mouseUp(const juce::MouseEvent& event) {
    const auto shouldTriggerClick = !event.mouseWasDraggedSinceMouseDown();

    if (pressed) {
        pressed = false;
        repaint();
    }

    if (shouldTriggerClick && onClick)
        onClick();
}

void RasterIconButton::mouseExit(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);

    if (!pressed)
        return;

    pressed = false;
    repaint();
}

void RasterIconButton::rebuildCachedImages() {
    if (imageBounds.isEmpty()) {
        cachedOffImage = {};
        cachedOnImage = {};
        return;
    }

    cachedOffImage = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::buttonOff).rescaled(
        imageBounds.getWidth(),
        imageBounds.getHeight(),
        juce::Graphics::highResamplingQuality);
    cachedOnImage = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::buttonOn).rescaled(
        imageBounds.getWidth(),
        imageBounds.getHeight(),
        juce::Graphics::highResamplingQuality);
}
