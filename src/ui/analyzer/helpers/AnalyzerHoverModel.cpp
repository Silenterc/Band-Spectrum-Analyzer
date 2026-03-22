#include "AnalyzerHoverModel.h"

#include <algorithm>
#include <cmath>

namespace {
    // The hover readout is intentionally calibrated slightly below the raw geometric
    // mapping because the on-screen cursor alignment reads about 0.5 dB high in use.
    constexpr float hoverVolumeCalibrationDb = 0.5f;
}

AnalyzerHoverModel::AnalyzerHoverModel(const AnalyzerGeometry &geometryToUse, const FrequencyFormatter &formatterToUse,
                                       const MusicTheory &musicTheoryToUse)
    : geometry(geometryToUse), formatter(formatterToUse), musicTheory(musicTheoryToUse) {
}

std::optional<AnalyzerHoverInfo> AnalyzerHoverModel::build(const juce::Rectangle<float> &localBounds,
                                                           const juce::Rectangle<float> &plotBounds,
                                                           const std::vector<Analyzer::BandInfo> &bandInfo,
                                                           const float gridMinDb,
                                                           const float gridMaxDb,
                                                           float visibleMinFrequencyHz, float visibleMaxFrequencyHz,
                                                           juce::Point<float> hoverPosition) const {
    const auto bandIndex = geometry.bandIndexAt(hoverPosition, bandInfo.size(), plotBounds);

    if (!bandIndex.has_value())
        return std::nullopt;

    const auto hoveredDb = geometry.dbForY(hoverPosition.y, gridMinDb, gridMaxDb, plotBounds) + hoverVolumeCalibrationDb;
    // Hover frequency follows the cursor on the log axis, not the nearest band center
    const auto hoveredFrequencyHz = std::round(
        geometry.frequencyForX(hoverPosition.x, visibleMinFrequencyHz, visibleMaxFrequencyHz, plotBounds));

    AnalyzerHoverInfo hoverInfo;
    hoverInfo.bounds = geometry.getTooltipBounds(hoverPosition, plotBounds, localBounds);
    hoverInfo.bandIndex = *bandIndex;
    hoverInfo.lines[hoverInfo.lineCount++] = "Volume: " + formatter.formatDecibels(hoveredDb);
    hoverInfo.lines[hoverInfo.lineCount++] = formatter.formatHoverFrequency(hoveredFrequencyHz);
    hoverInfo.lines[hoverInfo.lineCount++] = musicTheory.getNearestNoteName(hoveredFrequencyHz);
    return hoverInfo;
}
