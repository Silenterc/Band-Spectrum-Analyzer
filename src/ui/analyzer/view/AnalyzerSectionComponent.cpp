#include "AnalyzerSectionComponent.h"

#include "../helpers/AnalyzerGeometry.h"

AnalyzerSectionComponent::AnalyzerSectionComponent(AnalyzerDataSource& dataSource, const Ui::Theme& themeToUse)
    : theme(themeToUse),
      analyzerDisplayComponent(dataSource, themeToUse) {
    addAndMakeVisible(analyzerDisplayComponent);
}

AnalyzerSectionComponent::~AnalyzerSectionComponent() = default;

void AnalyzerSectionComponent::paint(juce::Graphics& g) {
    juce::ignoreUnused(g);
}

void AnalyzerSectionComponent::resized() {
    const auto layout = computeLayout();
    analyzerDisplayComponent.setBounds(layout.displayBounds);
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
