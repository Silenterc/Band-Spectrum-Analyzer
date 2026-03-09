#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/AnalyzerEngine.h"
#include "AnalyzerGeometry.h"
#include "AnalyzerHoverModel.h"
#include "FrequencyFormatter.h"
#include "MusicTheory.h"

/**
 * One horizontal dB grid marker
 */
struct AnalyzerGridLine {
    // Y position inside the component
    float y = 0.0f;
    // Label shown on the left axis
    juce::String label;
};

/**
 * One vertical frequency scale marker
 */
struct AnalyzerFrequencyMarker {
    // X position inside the plot
    float x = 0.0f;
    // Label shown under the plot
    juce::String label;
};

/**
 * Draw-ready bar state for one analyzer band
 */
struct AnalyzerBarModel {
    // Screen bounds for the bar
    juce::Rectangle<float> bounds;
    // Current displayed level for that bar
    float displayedDb = 0.0f;
    // Whether this bar is currently hovered
    bool isHovered = false;
};

/**
 * Builds draw-ready analyzer UI state from the latest snapshot
 */
class AnalyzerViewModel final {
public:
    AnalyzerViewModel();

    /**
     * Rebuilds the view model from the latest analyzer snapshot
     */
    void update(const Analyzer::Snapshot &snapshot, float gridMinDb, float gridMaxDb, float gridStepDb,
                const juce::Rectangle<float> &localBounds, const std::optional<juce::Point<float>> &hoverPosition);

    /**
     * Returns the current plot area
     */
    const juce::Rectangle<float> &getPlotBounds() const;

    /**
     * Returns the current dB grid markers
     */
    const std::vector<AnalyzerGridLine> &getGridLines() const;

    /**
     * Returns the current frequency scale markers
     */
    const std::vector<AnalyzerFrequencyMarker> &getFrequencyMarkers() const;

    /**
     * Returns the current draw-ready bar models
     */
    const std::vector<AnalyzerBarModel> &getBars() const;

    /**
     * Returns the current hover tooltip if any
     */
    const std::optional<AnalyzerHoverInfo> &getHoverInfo() const;

    /**
     * Returns the current minimum visible dB
     */
    float getGridMinDb() const;

private:
    void updateGrid(const std::vector<Analyzer::BandInfo> &bandInfo, float gridMinDb, float gridMaxDb, float gridStepDb);
    void updateBars(const Analyzer::Snapshot &snapshot, float gridMinDb, float gridMaxDb,
                    const std::optional<juce::Point<float>> &hoverPosition);
    static float getDisplayedLevelDb(size_t bandIndex, const Analyzer::Frame &latestFrame, float gridMinDb) ;

    AnalyzerGeometry geometry;
    FrequencyFormatter formatter;
    MusicTheory musicTheory;
    AnalyzerHoverModel hoverModel;

    juce::Rectangle<float> plotBounds;
    std::vector<AnalyzerGridLine> gridLines;
    std::vector<AnalyzerFrequencyMarker> frequencyMarkers;
    std::vector<AnalyzerBarModel> bars;
    std::optional<AnalyzerHoverInfo> hoverInfo;
    float currentGridMinDb = 0.0f;
};
