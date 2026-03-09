#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/AnalyzerData.h"
#include "AnalyzerGeometry.h"
#include "FrequencyFormatter.h"
#include "MusicTheory.h"

/**
 * Draw-ready hover tooltip state
 */
struct AnalyzerHoverInfo {
    // Tooltip bounds inside the component
    juce::Rectangle<float> bounds;
    // Hovered band index used for highlighting
    int bandIndex = -1;
    // Current displayed level text
    juce::String levelText;
    // Current cursor frequency text
    juce::String frequencyText;
    // Nearest note name for the cursor frequency
    juce::String noteText;
};

/**
 * Builds hover tooltip state from analyzer data and mouse position
 */
class AnalyzerHoverModel final {
public:
    AnalyzerHoverModel(const AnalyzerGeometry &geometry, const FrequencyFormatter &formatter,
                       const MusicTheory &musicTheory);

    /**
     * Returns the hover info for the current mouse position
     */
    [[nodiscard]] std::optional<AnalyzerHoverInfo> build(const juce::Rectangle<float> &localBounds,
                                                         const juce::Rectangle<float> &plotBounds,
                                                         const std::vector<Analyzer::BandInfo> &bandInfo,
                                                         const Analyzer::Frame &latestFrame, float gridMinDb,
                                                         float visibleMinFrequencyHz, float visibleMaxFrequencyHz,
                                                         juce::Point<float> hoverPosition) const;

private:
    static float getDisplayedLevelDb(size_t bandIndex, const Analyzer::Frame &latestFrame, float gridMinDb);

    const AnalyzerGeometry &geometry;
    const FrequencyFormatter &formatter;
    const MusicTheory &musicTheory;
};
