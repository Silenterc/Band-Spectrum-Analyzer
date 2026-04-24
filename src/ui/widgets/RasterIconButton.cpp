#include "RasterIconButton.h"

#include "ui/theme/UiRasterAssets.h"

RasterIconButton::RasterIconButton(const Ui::Theme& themeToUse)
    : juce::Button({}),
      theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTriggeredOnMouseDown(false);
    setWantsKeyboardFocus(true);
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

void RasterIconButton::paintButton(juce::Graphics& g, const bool isMouseOverButton, const bool isButtonDown) {
    juce::ignoreUnused(isMouseOverButton);
    const auto& buttonImage = (active || isButtonDown) ? cachedOnImage : cachedOffImage;
    const auto alpha = isEnabled() ? 1.0f : 0.45f;
    g.setOpacity(alpha);
    if (buttonImage.isValid())
        g.drawImageAt(buttonImage, imageBounds.getX(), imageBounds.getY());

    const auto iconColour = (active || isButtonDown) ? theme.hardwareMarkingDark : theme.hardwareMarkingLight;
    Ui::drawIcon(g, icon, iconBounds.toFloat(), iconColour);
    g.setOpacity(1.0f);
}

void RasterIconButton::resized() {
    imageBounds = Ui::getScaledAssetBoundsWithin(Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::buttonOff),
                                                 theme.metrics.assets.rasterScale,
                                                 getLocalBounds(),
                                                 scaleMultiplier);
    iconBounds = Ui::getScaledInnerBounds(imageBounds, 0.28f, iconScaleMultiplier);
    rebuildCachedImages();
}

void RasterIconButton::buttonStateChanged() {
    juce::Button::buttonStateChanged();

    const auto isCurrentlyDown = isDown();
    if (!wasDown && isCurrentlyDown && onPressed)
        onPressed();

    wasDown = isCurrentlyDown;
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
