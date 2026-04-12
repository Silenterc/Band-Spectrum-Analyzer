#include "AnalyzerHoverModel.h"

#include <algorithm>
#include <cmath>

AnalyzerHoverModel::AnalyzerHoverModel(const AnalyzerGeometry &geometryToUse, const FrequencyFormatter &formatterToUse,
                                       const MusicTheory &musicTheoryToUse)
    : geometry(geometryToUse),
      formatter(formatterToUse),
      musicTheory(musicTheoryToUse) {
}

std::optional<AnalyzerHoverInfo> AnalyzerHoverModel::build(const juce::Rectangle<float> &localBounds,
                                                           const juce::Rectangle<float> &plotBounds,
                                                           const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                                                           const float gridMinDb,
                                                           const float gridMaxDb,
                                                           float visibleMinFrequencyHz, float visibleMaxFrequencyHz,
                                                           juce::Point<float> hoverPosition) const {
    std::optional<size_t> bandIndex;
    if (plotBounds.contains(hoverPosition)) {
        for (size_t visibleBandIndex = 0; visibleBandIndex < visibleBands.size(); ++visibleBandIndex) {
            if (visibleBands[visibleBandIndex].hitBounds.contains(hoverPosition)) {
                bandIndex = visibleBandIndex;
                break;
            }
        }
    }

    if (!bandIndex.has_value())
        return std::nullopt;

    const auto hoveredDb = geometry.dbForY(hoverPosition.y - geometry.getTooltipCursorReferenceOffsetY(),
                                           gridMinDb,
                                           gridMaxDb,
                                           plotBounds);
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
