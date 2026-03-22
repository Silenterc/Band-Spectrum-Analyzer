#include "AnalyzerSectionComponent.h"

#include <BinaryData.h>

#include "../helpers/AnalyzerGeometry.h"

AnalyzerSectionComponent::AnalyzerSectionComponent(AnalyzerDataSource& dataSource, const Ui::Theme& themeToUse)
    : theme(themeToUse),
      analyzerDisplayComponent(dataSource, themeToUse) {
    setOpaque(true);
    addAndMakeVisible(analyzerDisplayComponent);
}

AnalyzerSectionComponent::~AnalyzerSectionComponent() = default;

void AnalyzerSectionComponent::paint(juce::Graphics& g) {
    g.drawImageAt(cachedBackground, 0, 0);
}

void AnalyzerSectionComponent::resized() {
    const auto layout = computeLayout();
    analyzerDisplayComponent.setBounds(layout.displayBounds);
    rebuildCachedBackground();
}

AnalyzerSectionComponent::Layout AnalyzerSectionComponent::computeLayout() const {
    Layout layout;
    const auto targetPlotBounds = getLocalBounds().reduced(theme.metrics.analyzerSection.plotInset);
    const auto& plotMargins = AnalyzerLayout::plotMargins;
    layout.displayBounds = juce::Rectangle<int>(
        juce::roundToInt(targetPlotBounds.getX() - plotMargins.left),
        juce::roundToInt(targetPlotBounds.getY() - plotMargins.top),
        juce::roundToInt(targetPlotBounds.getWidth() + plotMargins.left + plotMargins.right),
        juce::roundToInt(targetPlotBounds.getHeight() + plotMargins.top + plotMargins.bottom));
    return layout;
}

void AnalyzerSectionComponent::rebuildCachedBackground() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        cachedBackground = {};
        return;
    }

    cachedBackground = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics graphics(cachedBackground);
    const auto& backgroundImage = getBackgroundImage();
    graphics.drawImage(backgroundImage,
                       bounds.getX(),
                       bounds.getY(),
                       bounds.getWidth(),
                       bounds.getHeight(),
                       0,
                       0,
                       backgroundImage.getWidth(),
                       backgroundImage.getHeight());
}

const juce::Image& AnalyzerSectionComponent::getBackgroundImage() {
    static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::background_2_png,
                                                              static_cast<size_t>(BinaryData::background_2_pngSize));
    return image;
}
