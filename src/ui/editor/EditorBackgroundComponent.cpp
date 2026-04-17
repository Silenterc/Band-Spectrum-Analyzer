#include "EditorBackgroundComponent.h"

#include "../theme/UiRasterAssets.h"

EditorBackgroundComponent::EditorBackgroundComponent(const Ui::Theme&) {
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
}
