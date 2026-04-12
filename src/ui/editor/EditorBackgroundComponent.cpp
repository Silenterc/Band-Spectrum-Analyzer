#include "EditorBackgroundComponent.h"

#include "../theme/UiRasterAssets.h"

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

    const auto& background = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::background);
    graphics.drawImage(background,
                       bounds.getX(),
                       bounds.getY(),
                       bounds.getWidth(),
                       bounds.getHeight(),
                       0,
                       0,
                       background.getWidth(),
                       background.getHeight());

    const auto& screw = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::screw);
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
