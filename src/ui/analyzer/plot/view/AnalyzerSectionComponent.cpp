#include "ui/analyzer/plot/view/AnalyzerSectionComponent.h"

#include "ui/theme/UiRasterAssets.h"
#include "ui/analyzer/plot/logic/AnalyzerGeometry.h"

AnalyzerSectionComponent::AnalyzerSectionComponent(AnalyzerRawTraceSource& rawTraceSource,
                                                   AnalyzerUiSnapshotSource& snapshotSource,
                                                   const Ui::Theme& themeToUse)
    : theme(themeToUse),
      analyzerDisplayComponent(rawTraceSource, snapshotSource, themeToUse) {
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
    const auto& plotMargins = theme.metrics.analyzerPlot;
    layout.displayBounds = juce::Rectangle<int>(
        juce::roundToInt(targetPlotBounds.getX() - plotMargins.plotMarginLeft),
        juce::roundToInt(targetPlotBounds.getY() - plotMargins.plotMarginTop),
        juce::roundToInt(targetPlotBounds.getWidth() + plotMargins.plotMarginLeft + plotMargins.plotMarginRight),
        juce::roundToInt(targetPlotBounds.getHeight() + plotMargins.plotMarginTop + plotMargins.plotMarginBottom));
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
}
