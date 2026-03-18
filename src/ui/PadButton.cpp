#include "PadButton.h"

#include <BinaryData.h>

#include "UiIcons.h"

PadButton::PadButton(const Ui::Theme& themeToUse, juce::String labelText)
    : theme(themeToUse), label(std::move(labelText)) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

int PadButton::getPreferredHeight(const int availableWidth) const {
    const auto& metrics = theme.metrics.meterControls;
    const auto contentWidth = juce::jmax(1, availableWidth - metrics.horizontalPadding * 2);
    const auto& offImage = getOffImage();
    const auto rasterScale = theme.metrics.assets.rasterScale;
    const auto logicalWidth = static_cast<float>(offImage.getWidth()) / rasterScale;
    const auto logicalHeight = static_cast<float>(offImage.getHeight()) / rasterScale;
    const auto widthScale = static_cast<float>(contentWidth) / logicalWidth;
    const auto scale = juce::jlimit(0.0f, 1.0f, widthScale) * metrics.padScale;
    const auto targetHeight = juce::jmax(1, juce::roundToInt(logicalHeight * scale));
    return targetHeight;
}

bool PadButton::hitTest(const int x, const int y) {
    return padBounds.contains(x, y);
}

void PadButton::paint(juce::Graphics& g) {
    const auto& padImage = active ? cachedOnImage : cachedOffImage;
    if (padImage.isValid())
        g.drawImageAt(padImage, padBounds.getX(), padBounds.getY());

    const auto markingColour = active ? theme.hardwareMarkingDark : theme.hardwareMarkingLight;

    if (overlayIcon == OverlayIcon::snowflake) {
        const auto iconInset = static_cast<float>(padBounds.getWidth()) * 0.28f;
        Ui::drawSnowflakeIcon(g, padBounds.toFloat().reduced(iconInset), markingColour);
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

    const auto& offImage = getOffImage();
    const auto rasterScale = theme.metrics.assets.rasterScale;
    const auto logicalWidth = static_cast<float>(offImage.getWidth()) / rasterScale;
    const auto logicalHeight = static_cast<float>(offImage.getHeight()) / rasterScale;

    const auto widthScale = static_cast<float>(bounds.getWidth()) / logicalWidth;
    const auto heightScale = static_cast<float>(bounds.getHeight()) / logicalHeight;
    const auto scale = juce::jlimit(0.0f, 1.0f, std::min(widthScale, heightScale)) * metrics.padScale;

    const auto targetWidth = juce::jmax(1, juce::roundToInt(logicalWidth * scale));
    const auto targetHeight = juce::jmax(1, juce::roundToInt(logicalHeight * scale));
    padBounds = juce::Rectangle<int>(targetWidth, targetHeight).withCentre(bounds.getCentre());

    rebuildCachedPadImages();
}

void PadButton::setActive(const bool shouldBeActive) {
    if (active == shouldBeActive)
        return;

    active = shouldBeActive;
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

const juce::Image& PadButton::getOffImage() {
    static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::pad_off_png,
                                                              static_cast<size_t>(BinaryData::pad_off_pngSize));
    return image;
}

const juce::Image& PadButton::getOnImage() {
    static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::pad_on_png,
                                                              static_cast<size_t>(BinaryData::pad_on_pngSize));
    return image;
}

void PadButton::rebuildCachedPadImages() {
    if (padBounds.isEmpty()) {
        cachedOffImage = {};
        cachedOnImage = {};
        return;
    }

    cachedOffImage = getOffImage().rescaled(padBounds.getWidth(), padBounds.getHeight(), juce::Graphics::highResamplingQuality);
    cachedOnImage = getOnImage().rescaled(padBounds.getWidth(), padBounds.getHeight(), juce::Graphics::highResamplingQuality);
}
