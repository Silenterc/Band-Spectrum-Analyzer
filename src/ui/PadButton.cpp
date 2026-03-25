#include "PadButton.h"

#include "UiIcons.h"
#include "UiRasterAssets.h"

PadButton::PadButton(const Ui::Theme& themeToUse, juce::String labelText)
    : theme(themeToUse), label(std::move(labelText)) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

int PadButton::getPreferredHeight(const int availableWidth) const {
    const auto& metrics = theme.metrics.meterControls;
    const auto contentWidth = juce::jmax(1, availableWidth - metrics.horizontalPadding * 2);
    return getTargetPadBounds({0, 0, contentWidth, 0}).getHeight();
}

bool PadButton::hitTest(const int x, const int y) {
    return (drawsPad ? padBounds : overlayIconBounds).contains(x, y);
}

void PadButton::paint(juce::Graphics& g) {
    const auto& padImage = active ? cachedOnImage : cachedOffImage;
    if (drawsPad && padImage.isValid())
        g.drawImageAt(padImage, padBounds.getX(), padBounds.getY());

    const auto markingColour = active
                                   ? activeMarkingColourOverride.value_or(theme.hardwareMarkingDark)
                                   : theme.hardwareMarkingLight;

    if (overlayIcon != OverlayIcon::none) {
        switch (overlayIcon) {
            case OverlayIcon::none:
                break;
            case OverlayIcon::power:
                Ui::drawPowerIcon(g, overlayIconBounds.toFloat(), markingColour);
                break;
            case OverlayIcon::settings:
                Ui::drawSettingsIcon(g, overlayIconBounds.toFloat(), markingColour);
                break;
            case OverlayIcon::snowflake:
                Ui::drawSnowflakeIcon(g, overlayIconBounds.toFloat(), markingColour);
                break;
        }
    }

    if (label.isEmpty())
        return;

    g.setColour(markingColour);
    g.setFont(juce::FontOptions(theme.metrics.meterControls.padTextFontHeight).withStyle("Bold"));
    g.drawText(label, padBounds, juce::Justification::centred, false);
}

void PadButton::resized() {
    const auto& metrics = theme.metrics.meterControls;
    auto bounds = getLocalBounds().reduced(metrics.horizontalPadding, 0);
    padBounds = getTargetPadBounds(bounds);
    overlayIconBounds = Ui::getScaledInnerBounds(padBounds, 0.28f, overlayIconScaleMultiplier);

    rebuildCachedPadImages();
}

void PadButton::setActive(const bool shouldBeActive) {
    if (active == shouldBeActive)
        return;

    active = shouldBeActive;
    repaint();
}

void PadButton::setActiveMarkingColour(const juce::Colour newActiveMarkingColour) {
    if (activeMarkingColourOverride.has_value() && activeMarkingColourOverride.value() == newActiveMarkingColour)
        return;

    activeMarkingColourOverride = newActiveMarkingColour;
    repaint(padBounds);
}

void PadButton::setAssetStyle(const AssetStyle newAssetStyle) {
    if (assetStyle == newAssetStyle)
        return;

    assetStyle = newAssetStyle;
    rebuildCachedPadImages();
    repaint();
}

void PadButton::setDrawsPad(const bool shouldDrawPad) {
    if (drawsPad == shouldDrawPad)
        return;

    drawsPad = shouldDrawPad;
    repaint();
}

void PadButton::setScaleMultiplier(const float newScaleMultiplier) {
    const auto clampedScaleMultiplier = juce::jmax(0.0f, newScaleMultiplier);
    if (juce::approximatelyEqual(scaleMultiplier, clampedScaleMultiplier))
        return;

    scaleMultiplier = clampedScaleMultiplier;
    resized();
    repaint();
}

void PadButton::setOverlayIconScaleMultiplier(const float newOverlayIconScaleMultiplier) {
    const auto clampedOverlayScaleMultiplier = juce::jmax(0.0f, newOverlayIconScaleMultiplier);
    if (juce::approximatelyEqual(overlayIconScaleMultiplier, clampedOverlayScaleMultiplier))
        return;

    overlayIconScaleMultiplier = clampedOverlayScaleMultiplier;
    resized();
    repaint();
}

void PadButton::setLabel(juce::String newLabel) {
    if (label == newLabel)
        return;

    label = std::move(newLabel);
    repaint(padBounds);
}

void PadButton::setOverlayIcon(const OverlayIcon newOverlayIcon) {
    if (overlayIcon == newOverlayIcon)
        return;

    overlayIcon = newOverlayIcon;
    repaint(padBounds);
}

void PadButton::mouseUp(const juce::MouseEvent& event) {
    if (!event.mouseWasDraggedSinceMouseDown() && onClick)
        onClick();
}

const juce::Image& PadButton::getResolvedOnImage() const {
    switch (assetStyle) {
        case AssetStyle::standard:
            return Ui::getRasterAsset(Ui::RasterAssetId::padOn);
        case AssetStyle::freeze:
            return Ui::getRasterAsset(Ui::RasterAssetId::padFreezeOn);
    }

    jassertfalse;
    return Ui::getRasterAsset(Ui::RasterAssetId::padOn);
}

juce::Rectangle<int> PadButton::getTargetPadBounds(const juce::Rectangle<int> availableBounds) const {
    return Ui::getScaledAssetBoundsWithin(Ui::getRasterAsset(Ui::RasterAssetId::padOff),
                                          theme.metrics.assets.rasterScale,
                                          availableBounds,
                                          theme.metrics.meterControls.padScale * scaleMultiplier);
}

void PadButton::rebuildCachedPadImages() {
    if (padBounds.isEmpty()) {
        cachedOffImage = {};
        cachedOnImage = {};
        return;
    }

    cachedOffImage = Ui::getRasterAsset(Ui::RasterAssetId::padOff).rescaled(padBounds.getWidth(),
                                                                             padBounds.getHeight(),
                                                                             juce::Graphics::highResamplingQuality);
    cachedOnImage = getResolvedOnImage().rescaled(padBounds.getWidth(), padBounds.getHeight(), juce::Graphics::highResamplingQuality);
}
