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
    const auto &dividerMetrics = theme.metrics.sectionDivider;
    juce::ColourGradient gradient(
        theme.sectionDividerShadow.withMultipliedAlpha(dividerMetrics.startAlpha),
        0.0f,
        0.0f,
        theme.sectionDividerHighlight.withMultipliedAlpha(dividerMetrics.endAlpha),
        orientation == Orientation::vertical ? static_cast<float>(bounds.getWidth()) : 0.0f,
        orientation == Orientation::horizontal ? static_cast<float>(bounds.getHeight()) : 0.0f,
        false);

    gradient.addColour(dividerMetrics.middleStartPosition,
                       theme.sectionDividerShadow.withMultipliedAlpha(dividerMetrics.middleStartAlpha));
    gradient.addColour(dividerMetrics.middleEndPosition,
                       theme.sectionDividerHighlight.withMultipliedAlpha(dividerMetrics.middleEndAlpha));

    graphics.setGradientFill(gradient);
    graphics.fillRect(bounds);
}
