#include "AnalyzerViewModel.h"

#include <algorithm>
#include <array>

AnalyzerViewModel::AnalyzerViewModel()
    : hoverModel(geometry, formatter, musicTheory) {
}

void AnalyzerViewModel::update(const Analyzer::Snapshot &snapshot, float gridMinDb, float gridMaxDb, float gridStepDb,
                               const juce::Rectangle<float> &localBounds,
                               const std::optional<juce::Point<float>> &hoverPositionToUse) {
    currentGridMinDb = gridMinDb;
    plotBounds = geometry.getPlotBounds(localBounds);
    updateGrid(snapshot.bandInfo, gridMinDb, gridMaxDb, gridStepDb);
    updateBars(snapshot, gridMinDb, gridMaxDb, hoverPositionToUse);

    if (hoverPositionToUse.has_value())
        hoverInfo = hoverModel.build(localBounds, plotBounds, snapshot.bandInfo, snapshot.frame, gridMinDb, *hoverPositionToUse);
    else
        hoverInfo.reset();
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

const std::vector<AnalyzerBarModel> &AnalyzerViewModel::getBars() const {
    return bars;
}

const std::optional<AnalyzerHoverInfo> &AnalyzerViewModel::getHoverInfo() const {
    return hoverInfo;
}

float AnalyzerViewModel::getGridMinDb() const {
    return currentGridMinDb;
}

void AnalyzerViewModel::updateGrid(const std::vector<Analyzer::BandInfo> &bandInfo, float gridMinDb, float gridMaxDb,
                                   float gridStepDb) {
    gridLines.clear();
    frequencyMarkers.clear();

    for (float db = gridMinDb; db <= gridMaxDb + 0.001f; db += gridStepDb) {
        AnalyzerGridLine gridLine;
        gridLine.y = geometry.yForDb(db, gridMinDb, gridMaxDb, plotBounds);
        gridLine.label = juce::String(static_cast<int>(std::round(db)));
        gridLines.push_back(gridLine);
    }

    static constexpr std::array<float, 10> frequencyLabelsHz{
        20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f
    };

    for (auto frequencyHz: frequencyLabelsHz) {
        AnalyzerFrequencyMarker frequencyMarker;
        frequencyMarker.x = geometry.xForFrequency(frequencyHz, bandInfo, plotBounds);
        frequencyMarker.label = formatter.formatScaleFrequency(frequencyHz);
        frequencyMarkers.push_back(frequencyMarker);
    }
}

void AnalyzerViewModel::updateBars(const Analyzer::Snapshot &snapshot, float gridMinDb, float gridMaxDb,
                                   const std::optional<juce::Point<float>> &hoverPositionToUse) {
    const auto hoveredBandIndex = hoverPositionToUse.has_value()
                                      ? geometry.bandIndexAt(*hoverPositionToUse, snapshot.bandInfo.size(), plotBounds)
                                      : -1;

    bars.resize(snapshot.bandInfo.size());

    for (size_t bandIndex = 0; bandIndex < snapshot.bandInfo.size(); ++bandIndex) {
        AnalyzerBarModel barModel;
        barModel.displayedDb = getDisplayedLevelDb(bandIndex, snapshot.frame, gridMinDb);
        barModel.bounds = geometry.getBarBounds(bandIndex, snapshot.bandInfo.size(), barModel.displayedDb, gridMinDb,
                                                gridMaxDb, plotBounds);
        barModel.isHovered = hoveredBandIndex == static_cast<int>(bandIndex);
        bars[bandIndex] = barModel;
    }
}

float AnalyzerViewModel::getDisplayedLevelDb(size_t bandIndex, const Analyzer::Frame &latestFrame, const float gridMinDb) {
    if (bandIndex >= latestFrame.peakDb.size() || bandIndex >= latestFrame.rmsDb.size())
        return gridMinDb;

    return std::max(latestFrame.peakDb[bandIndex], latestFrame.rmsDb[bandIndex]);
}
