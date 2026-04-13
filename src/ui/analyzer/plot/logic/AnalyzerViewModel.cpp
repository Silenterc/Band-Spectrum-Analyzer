#include "AnalyzerViewModel.h"

#include <cmath>

AnalyzerViewModel::AnalyzerViewModel(const Ui::Theme &themeToUse)
    : theme(themeToUse),
      geometry(themeToUse),
      hoverModel(geometry, formatter, musicTheory) {
}

void AnalyzerViewModel::updateLayout(const std::vector<Analyzer::BandInfo> &bandInfo,
                                     const AnalyzerViewState &viewState,
                                     const float gridMinDb,
                                     const float gridMaxDb,
                                     const float gridStepDb,
                                     const juce::Rectangle<float> &sectionBounds,
                                     const juce::Rectangle<float> &displayBounds) {
    layout = {};
    layout.sectionBounds = sectionBounds.toNearestInt().toFloat();
    layout.displayBounds = displayBounds.toNearestInt().toFloat();
    layout.plotBounds = geometry.getPlotBounds(layout.displayBounds).toNearestInt().toFloat();
    layout.plotLocalBounds = juce::Rectangle<float>(0.0f, 0.0f, layout.plotBounds.getWidth(), layout.plotBounds.getHeight());
    layout.gridMinDb = gridMinDb;
    layout.gridMaxDb = gridMaxDb;
    layout.gridStepDb = gridStepDb;
    layout.plotFrameBounds = layout.plotBounds.expanded(theme.metrics.analyzerPlot.frameExpansion);
    updateVisibleFrequencyRange(bandInfo, viewState);
    updateVisibleBands(bandInfo);
    updateGrid(gridMinDb, gridMaxDb, gridStepDb);
}

std::optional<AnalyzerHoverInfo> AnalyzerViewModel::buildHover(
    const std::optional<juce::Point<float>> &plotLocalHoverPosition) const {
    if (!plotLocalHoverPosition.has_value() || layout.sectionVisibleBands.empty())
        return std::nullopt;

    const auto sectionHoverPosition = plotLocalHoverPosition->translated(layout.plotBounds.getX(), layout.plotBounds.getY());
    return hoverModel.build(layout.sectionBounds,
                            layout.plotBounds,
                            layout.sectionVisibleBands,
                            layout.gridMinDb,
                            layout.gridMaxDb,
                            layout.visibleMinFrequencyHz,
                            layout.visibleMaxFrequencyHz,
                            sectionHoverPosition);
}

const AnalyzerSectionLayout &AnalyzerViewModel::getLayout() const {
    return layout;
}

void AnalyzerViewModel::updateGrid(float gridMinDb, float gridMaxDb, float gridStepDb) {
    layout.gridLines.clear();
    layout.frequencyMarkers.clear();
    layout.gridLines.reserve(static_cast<size_t>(std::ceil((gridMaxDb - gridMinDb) / gridStepDb)) + 1);
    layout.frequencyMarkers.reserve(theme.metrics.analyzerPlot.frequencyScaleLabelsHz.size());

    for (float db = gridMinDb; db <= gridMaxDb + 0.001f; db += gridStepDb) {
        AnalyzerGridLineLayout gridLine;
        gridLine.sectionY = geometry.yForDb(db, gridMinDb, gridMaxDb, layout.plotBounds);
        gridLine.plotLocalY = gridLine.sectionY - layout.plotBounds.getY();
        gridLine.label = juce::String(static_cast<int>(std::round(db)));
        layout.gridLines.push_back(gridLine);
    }

    for (auto frequencyHz: theme.metrics.analyzerPlot.frequencyScaleLabelsHz) {
        if (layout.useCustomFrequencyRange
            && (frequencyHz < layout.visibleMinFrequencyHz || frequencyHz > layout.visibleMaxFrequencyHz))
            continue;

        AnalyzerFrequencyMarkerLayout frequencyMarker;
        frequencyMarker.sectionX = geometry.xForFrequency(frequencyHz,
                                                          layout.visibleMinFrequencyHz,
                                                          layout.visibleMaxFrequencyHz,
                                                          layout.plotBounds);
        frequencyMarker.plotLocalX = frequencyMarker.sectionX - layout.plotBounds.getX();
        frequencyMarker.label = formatter.formatScaleFrequency(frequencyHz);
        layout.frequencyMarkers.push_back(frequencyMarker);
    }
}

void AnalyzerViewModel::updateVisibleBands(const std::vector<Analyzer::BandInfo> &bandInfo) {
    layout.sectionVisibleBands.clear();
    layout.plotVisibleBands.clear();
    layout.sectionVisibleBands.reserve(bandInfo.size());
    layout.plotVisibleBands.reserve(bandInfo.size());

    if (bandInfo.empty())
        return;

    const auto interBandGapPixels = theme.metrics.analyzerPlot.interBandGapPixels;

    std::vector<size_t> sourceBandIndices;
    sourceBandIndices.reserve(bandInfo.size());

    for (size_t bandIndex = 0; bandIndex < bandInfo.size(); ++bandIndex) {
        const auto &band = bandInfo[bandIndex];
        if (band.highHz < layout.visibleMinFrequencyHz || band.lowHz > layout.visibleMaxFrequencyHz)
            continue;

        sourceBandIndices.push_back(bandIndex);
    }

    layout.sectionVisibleBands.reserve(sourceBandIndices.size());
    layout.plotVisibleBands.reserve(sourceBandIndices.size());
    for (size_t visibleBandIndex = 0; visibleBandIndex < sourceBandIndices.size(); ++visibleBandIndex) {
        const auto sourceBandIndex = sourceBandIndices[visibleBandIndex];
        const auto &band = bandInfo[sourceBandIndex];

        AnalyzerVisibleBandLayout sectionBand;
        sectionBand.sourceBandIndex = sourceBandIndex;
        sectionBand.hitBounds = geometry.getBandHitBounds(band.lowHz,
                                                          band.highHz,
                                                          layout.visibleMinFrequencyHz,
                                                          layout.visibleMaxFrequencyHz,
                                                          layout.plotBounds);
        const auto drawLeft = static_cast<int>(std::round(sectionBand.hitBounds.getX()));
        auto drawRight = static_cast<int>(std::round(sectionBand.hitBounds.getRight()));
        if (visibleBandIndex + 1 != sourceBandIndices.size())
            drawRight -= interBandGapPixels;

        const auto drawWidth = juce::jmax(0, drawRight - drawLeft);
        sectionBand.drawBounds = juce::Rectangle<int>(drawLeft,
                                                      static_cast<int>(std::round(sectionBand.hitBounds.getY())),
                                                      drawWidth,
                                                      static_cast<int>(std::round(sectionBand.hitBounds.getHeight()))).toFloat();

        AnalyzerVisibleBandLayout plotBand;
        plotBand.sourceBandIndex = sourceBandIndex;
        plotBand.hitBounds = sectionBand.hitBounds.translated(-layout.plotBounds.getX(), -layout.plotBounds.getY());
        plotBand.drawBounds = sectionBand.drawBounds.translated(-layout.plotBounds.getX(), -layout.plotBounds.getY());

        layout.sectionVisibleBands.push_back(sectionBand);
        layout.plotVisibleBands.push_back(plotBand);
    }
}

void AnalyzerViewModel::updateVisibleFrequencyRange(const std::vector<Analyzer::BandInfo> &bandInfo,
                                                    const AnalyzerViewState &viewState) {
    layout.useCustomFrequencyRange = viewState.useCustomFrequencyRange;

    if (bandInfo.empty()) {
        layout.visibleMinFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMinFrequencyHz;
        layout.visibleMaxFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMaxFrequencyHz;
        return;
    }

    const auto fullMinFrequencyHz = bandInfo.front().lowHz;
    const auto fullMaxFrequencyHz = bandInfo.back().highHz;
    const auto uiMaxFrequencyHz = juce::jmin(fullMaxFrequencyHz, theme.metrics.analyzerPlot.maxUiFrequencyHz);

    if (!viewState.useCustomFrequencyRange) {
        layout.visibleMinFrequencyHz = fullMinFrequencyHz;
        layout.visibleMaxFrequencyHz = uiMaxFrequencyHz;
        return;
    }

    layout.visibleMinFrequencyHz = juce::jlimit(fullMinFrequencyHz, uiMaxFrequencyHz, viewState.visibleMinFrequencyHz);
    layout.visibleMaxFrequencyHz = juce::jlimit(layout.visibleMinFrequencyHz, uiMaxFrequencyHz, viewState.visibleMaxFrequencyHz);
}
