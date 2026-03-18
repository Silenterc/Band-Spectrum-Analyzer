#include "EditorBackgroundComponent.h"

#include <BinaryData.h>
#include "UiRasterAssets.h"

EditorBackgroundComponent::EditorBackgroundComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse) {
    setOpaque(true);
    setInterceptsMouseClicks(false, false);
}

void EditorBackgroundComponent::paint(juce::Graphics& g) {
    g.drawImageAt(cachedLayer, 0, 0);
}

void EditorBackgroundComponent::resized() {
    rebuildCachedLayer();
}

void EditorBackgroundComponent::rebuildCachedLayer() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        cachedLayer = {};
        return;
    }

    cachedLayer = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics graphics(cachedLayer);

    const auto& background = getBackgroundImage();
    graphics.drawImage(background,
                       bounds.getX(),
                       bounds.getY(),
                       bounds.getWidth(),
                       bounds.getHeight(),
                       0,
                       0,
                       background.getWidth(),
                       background.getHeight());

    const auto& screw = getScrewImage();
    const auto rasterScale = theme.metrics.assets.rasterScale;
    const auto screwPadding = theme.metrics.background.screwPadding;
    const auto topLeftBounds = Ui::getLogicalAssetBounds(screw, rasterScale, {screwPadding, screwPadding});
    const auto topRightBounds = Ui::getLogicalAssetBounds(
        screw,
        rasterScale,
        {bounds.getWidth() - screwPadding - topLeftBounds.getWidth(), screwPadding});

    Ui::drawAssetWithin(graphics, screw, topLeftBounds);
    Ui::drawAssetWithin(graphics, screw, topRightBounds);
}

const juce::Image& EditorBackgroundComponent::getBackgroundImage() {
    static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::background_png,
                                                              static_cast<size_t>(BinaryData::background_pngSize));
    return image;
}

const juce::Image& EditorBackgroundComponent::getScrewImage() {
    static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::screw_png,
                                                              static_cast<size_t>(BinaryData::screw_pngSize));
    return image;
}
