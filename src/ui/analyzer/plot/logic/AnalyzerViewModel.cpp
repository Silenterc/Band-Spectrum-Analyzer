#include "AnalyzerViewModel.h"

#include <cmath>

AnalyzerViewModel::AnalyzerViewModel(const Ui::Theme &themeToUse)
    : theme(themeToUse),
      geometry(themeToUse),
      hoverModel(geometry, formatter, musicTheory) {
}

void AnalyzerViewModel::updateStaticLayout(const std::vector<Analyzer::BandInfo> &bandInfo,
                                           const AnalyzerViewState &viewState,
                                           float gridMinDb, float gridMaxDb, float gridStepDb,
                                           const juce::Rectangle<float> &localBounds) {
    currentGridMinDb = gridMinDb;
    plotBounds = geometry.getPlotBounds(localBounds);
    updateVisibleFrequencyRange(bandInfo, viewState);
    updateVisibleBands(bandInfo);
    updateGrid(gridMinDb, gridMaxDb, gridStepDb);
}

void AnalyzerViewModel::updateHover(const float gridMinDb,
                                    const float gridMaxDb,
                                    const juce::Rectangle<float> &localBounds,
                                    const std::optional<juce::Point<float>> &hoverPositionToUse) {
    if (!hoverPositionToUse.has_value()) {
        hoverInfo.reset();
        return;
    }

    if (visibleBands.empty()) {
        hoverInfo.reset();
        return;
    }

    hoverInfo = hoverModel.build(localBounds, plotBounds, visibleBands, gridMinDb, gridMaxDb,
                                 visibleMinFrequencyHz, visibleMaxFrequencyHz,
                                 *hoverPositionToUse);
}

const juce::Rectangle<float> &AnalyzerViewModel::getPlotBounds() const {
    return plotBounds;
}

const std::vector<AnalyzerGridLine> &AnalyzerViewModel::getGridLines() const {
    return gridLines;
}

const std::vector<AnalyzerFrequencyMarker> &AnalyzerViewModel::getFrequencyMarkers() const {
    return frequencyMarkers;
}

const std::optional<AnalyzerHoverInfo> &AnalyzerViewModel::getHoverInfo() const {
    return hoverInfo;
}

float AnalyzerViewModel::getGridMinDb() const {
    return currentGridMinDb;
}

const std::vector<AnalyzerVisibleBandLayout> &AnalyzerViewModel::getVisibleBands() const {
    return visibleBands;
}

void AnalyzerViewModel::updateGrid(float gridMinDb, float gridMaxDb, float gridStepDb) {
    gridLines.clear();
    frequencyMarkers.clear();
    gridLines.reserve(static_cast<size_t>(std::ceil((gridMaxDb - gridMinDb) / gridStepDb)) + 1);
    frequencyMarkers.reserve(theme.metrics.analyzerPlot.frequencyScaleLabelsHz.size());

    for (float db = gridMinDb; db <= gridMaxDb + 0.001f; db += gridStepDb) {
        AnalyzerGridLine gridLine;
        gridLine.y = geometry.yForDb(db, gridMinDb, gridMaxDb, plotBounds);
        gridLine.label = juce::String(static_cast<int>(std::round(db)));
        gridLines.push_back(gridLine);
    }

    for (auto frequencyHz: theme.metrics.analyzerPlot.frequencyScaleLabelsHz) {
        if (usingCustomFrequencyRange
            && (frequencyHz < visibleMinFrequencyHz || frequencyHz > visibleMaxFrequencyHz))
            continue;

        AnalyzerFrequencyMarker frequencyMarker;
        frequencyMarker.x = geometry.xForFrequency(frequencyHz, visibleMinFrequencyHz, visibleMaxFrequencyHz, plotBounds);
        frequencyMarker.label = formatter.formatScaleFrequency(frequencyHz);
        frequencyMarkers.push_back(frequencyMarker);
    }
}

void AnalyzerViewModel::updateVisibleBands(const std::vector<Analyzer::BandInfo> &bandInfo) {
    visibleBands.clear();
    visibleBands.reserve(bandInfo.size());

    if (bandInfo.empty())
        return;

    const auto interBandGapPixels = theme.metrics.analyzerPlot.interBandGapPixels;

    std::vector<size_t> sourceBandIndices;
    sourceBandIndices.reserve(bandInfo.size());

    for (size_t bandIndex = 0; bandIndex < bandInfo.size(); ++bandIndex) {
        const auto &band = bandInfo[bandIndex];
        if (band.highHz < visibleMinFrequencyHz || band.lowHz > visibleMaxFrequencyHz)
            continue;

        sourceBandIndices.push_back(bandIndex);
    }

    visibleBands.reserve(sourceBandIndices.size());
    for (size_t visibleBandIndex = 0; visibleBandIndex < sourceBandIndices.size(); ++visibleBandIndex) {
        const auto sourceBandIndex = sourceBandIndices[visibleBandIndex];
        const auto &band = bandInfo[sourceBandIndex];

        AnalyzerVisibleBandLayout visibleBand;
        visibleBand.sourceBandIndex = sourceBandIndex;
        visibleBand.hitBounds = geometry.getBandHitBounds(band.lowHz, band.highHz,
                                                          visibleMinFrequencyHz, visibleMaxFrequencyHz,
                                                          plotBounds);
        const auto drawLeft = static_cast<int>(std::round(visibleBand.hitBounds.getX()));
        auto drawRight = static_cast<int>(std::round(visibleBand.hitBounds.getRight()));
        if (visibleBandIndex + 1 != sourceBandIndices.size())
            drawRight -= interBandGapPixels;

        const auto drawWidth = juce::jmax(0, drawRight - drawLeft);
        visibleBand.drawBounds = juce::Rectangle<int>(drawLeft,
                                                      static_cast<int>(std::round(visibleBand.hitBounds.getY())),
                                                      drawWidth,
                                                      static_cast<int>(std::round(visibleBand.hitBounds.getHeight()))).toFloat();
        visibleBands.push_back(visibleBand);
    }
}
void AnalyzerViewModel::updateVisibleFrequencyRange(const std::vector<Analyzer::BandInfo> &bandInfo,
                                                    const AnalyzerViewState &viewState) {
    usingCustomFrequencyRange = viewState.useCustomFrequencyRange;

    if (bandInfo.empty()) {
        visibleMinFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMinFrequencyHz;
        visibleMaxFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMaxFrequencyHz;
        return;
    }

    const auto fullMinFrequencyHz = bandInfo.front().lowHz;
    const auto fullMaxFrequencyHz = bandInfo.back().highHz;
    const auto uiMaxFrequencyHz = juce::jmin(fullMaxFrequencyHz, theme.metrics.analyzerPlot.maxUiFrequencyHz);

    if (!viewState.useCustomFrequencyRange) {
        visibleMinFrequencyHz = fullMinFrequencyHz;
        visibleMaxFrequencyHz = uiMaxFrequencyHz;
        return;
    }

    visibleMinFrequencyHz = juce::jlimit(fullMinFrequencyHz, uiMaxFrequencyHz, viewState.visibleMinFrequencyHz);
    visibleMaxFrequencyHz = juce::jlimit(visibleMinFrequencyHz, uiMaxFrequencyHz, viewState.visibleMaxFrequencyHz);
}
