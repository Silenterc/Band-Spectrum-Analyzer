#include "AnalyzerHoverModel.h"

#include <algorithm>
#include <cmath>

AnalyzerHoverModel::AnalyzerHoverModel(const AnalyzerGeometry &geometryToUse, const FrequencyFormatter &formatterToUse,
                                       const MusicTheory &musicTheoryToUse)
    : geometry(geometryToUse), formatter(formatterToUse), musicTheory(musicTheoryToUse) {
}

std::optional<AnalyzerHoverInfo> AnalyzerHoverModel::build(const juce::Rectangle<float> &localBounds,
                                                           const juce::Rectangle<float> &plotBounds,
                                                           const std::vector<Analyzer::BandInfo> &bandInfo,
                                                           const Analyzer::Frame &latestFrame, float gridMinDb,
                                                           juce::Point<float> hoverPosition) const {
    const auto bandIndex = geometry.bandIndexAt(hoverPosition, bandInfo.size(), plotBounds);

    if (bandIndex < 0)
        return std::nullopt;

    const auto hoveredFrequencyHz = std::round(geometry.frequencyForX(hoverPosition.x, bandInfo, plotBounds));

    AnalyzerHoverInfo hoverInfo;
    hoverInfo.bounds = geometry.getTooltipBounds(hoverPosition, plotBounds, localBounds);
    hoverInfo.bandIndex = bandIndex;
    hoverInfo.levelText = formatter.formatDecibels(getDisplayedLevelDb(static_cast<size_t>(bandIndex), latestFrame, gridMinDb));
    hoverInfo.frequencyText = formatter.formatHoverFrequency(hoveredFrequencyHz);
    hoverInfo.noteText = musicTheory.getNearestNoteName(hoveredFrequencyHz);
    return hoverInfo;
}

float AnalyzerHoverModel::getDisplayedLevelDb(size_t bandIndex, const Analyzer::Frame &latestFrame, float gridMinDb) {
    if (bandIndex >= latestFrame.peakDb.size() || bandIndex >= latestFrame.rmsDb.size())
        return gridMinDb;

    return std::max(latestFrame.peakDb[bandIndex], latestFrame.rmsDb[bandIndex]);
}
