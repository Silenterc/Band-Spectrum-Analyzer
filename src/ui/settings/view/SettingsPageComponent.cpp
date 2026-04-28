#include "ui/settings/view/SettingsPageComponent.h"

#include "ui/theme/UiRasterAssets.h"

SettingsPageComponent::SettingsPageComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse) {
    setOpaque(true);
}

void SettingsPageComponent::paint(juce::Graphics& g) {
    if (cachedBackground.isValid())
        g.drawImageAt(cachedBackground, 0, 0);
}

void SettingsPageComponent::resized() {
    rebuildCachedBackground();
}

void SettingsPageComponent::rebuildCachedBackground() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        cachedBackground = {};
        return;
    }

    cachedBackground = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics graphics(cachedBackground);

    const auto& backgroundImage = Ui::getAnalyzerRasterAsset(Ui::AnalyzerRasterAssetId::background2);
    graphics.drawImage(backgroundImage,
                       bounds.getX(),
                       bounds.getY(),
                       bounds.getWidth(),
                       bounds.getHeight(),
                       0,
                       0,
                       backgroundImage.getWidth(),
                       backgroundImage.getHeight());

    Ui::drawTopCornerScrews(graphics, bounds, theme);
}
