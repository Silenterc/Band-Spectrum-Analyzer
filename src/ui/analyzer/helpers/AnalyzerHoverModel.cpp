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
                                                           const Analyzer::RenderFrame &renderFrame,
                                                           const Analyzer::MeterSettings &meterSettings, float gridMinDb,
                                                           float visibleMinFrequencyHz, float visibleMaxFrequencyHz,
                                                           juce::Point<float> hoverPosition) const {
    const auto bandIndex = geometry.bandIndexAt(hoverPosition, bandInfo.size(), plotBounds);

    if (!bandIndex.has_value())
        return std::nullopt;

    // Hover frequency follows the cursor on the log axis, not the nearest band center
    const auto hoveredFrequencyHz = std::round(
        geometry.frequencyForX(hoverPosition.x, visibleMinFrequencyHz, visibleMaxFrequencyHz, plotBounds));

    AnalyzerHoverInfo hoverInfo;
    hoverInfo.bounds = geometry.getTooltipBounds(hoverPosition, plotBounds, localBounds);
    hoverInfo.bandIndex = *bandIndex;

    if (meterSettings.showPeak)
        hoverInfo.lines[hoverInfo.lineCount++] =
            "Peak: " + formatter.formatDecibels(getPeakDb(*bandIndex, renderFrame, gridMinDb));

    if (meterSettings.showRms)
        hoverInfo.lines[hoverInfo.lineCount++] =
            "RMS:  " + formatter.formatDecibels(getRmsDb(*bandIndex, renderFrame, gridMinDb));

    hoverInfo.lines[hoverInfo.lineCount++] = formatter.formatHoverFrequency(hoveredFrequencyHz);
    hoverInfo.lines[hoverInfo.lineCount++] = musicTheory.getNearestNoteName(hoveredFrequencyHz);
    return hoverInfo;
}

float AnalyzerHoverModel::getPeakDb(size_t bandIndex, const Analyzer::RenderFrame &renderFrame, float gridMinDb) {
    if (bandIndex >= renderFrame.peakDb.size())
        return gridMinDb;

    return renderFrame.peakDb[bandIndex];
}

float AnalyzerHoverModel::getRmsDb(size_t bandIndex, const Analyzer::RenderFrame &renderFrame, float gridMinDb) {
    if (bandIndex >= renderFrame.rmsDb.size())
        return gridMinDb;

    return renderFrame.rmsDb[bandIndex];
}
