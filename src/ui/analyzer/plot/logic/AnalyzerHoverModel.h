#pragma once

#include <array>
#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "dsp/core/AnalyzerData.h"
#include "ui/analyzer/plot/logic/AnalyzerVisibleBandLayout.h"
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
    size_t bandIndex = 0;
    // Prebuilt tooltip lines drawn directly by the component
    std::array<juce::String, 4> lines;
    // Number of valid lines stored in `lines`
    size_t lineCount = 0;
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
                                                         const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                                                         float gridMinDb,
                                                         float gridMaxDb,
                                                         float visibleMinFrequencyHz, float visibleMaxFrequencyHz,
                                                         juce::Point<float> hoverPosition) const;

    const AnalyzerGeometry &geometry;
    const FrequencyFormatter &formatter;
    const MusicTheory &musicTheory;
};
