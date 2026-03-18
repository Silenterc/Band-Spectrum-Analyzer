#include "SectionDividerComponent.h"

SectionDividerComponent::SectionDividerComponent(const Ui::Theme& themeToUse, const Orientation orientationToUse)
    : theme(themeToUse), orientation(orientationToUse) {
    setInterceptsMouseClicks(false, false);
}

void SectionDividerComponent::paint(juce::Graphics& g) {
    g.drawImageAt(cachedLayer, 0, 0);
}

void SectionDividerComponent::resized() {
    rebuildCachedLayer();
}

void SectionDividerComponent::rebuildCachedLayer() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        cachedLayer = {};
        return;
    }

    cachedLayer = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics graphics(cachedLayer);
    juce::ColourGradient gradient(
        theme.sectionDividerShadow.withMultipliedAlpha(0.90f),
        0.0f,
        0.0f,
        theme.sectionDividerHighlight.withMultipliedAlpha(0.46f),
        orientation == Orientation::vertical ? static_cast<float>(bounds.getWidth()) : 0.0f,
        orientation == Orientation::horizontal ? static_cast<float>(bounds.getHeight()) : 0.0f,
        false);

    gradient.addColour(0.38, theme.sectionDividerShadow.withMultipliedAlpha(0.78f));
    gradient.addColour(0.74, theme.sectionDividerHighlight.withMultipliedAlpha(0.26f));

    graphics.setGradientFill(gradient);
    graphics.fillRect(bounds);
}
