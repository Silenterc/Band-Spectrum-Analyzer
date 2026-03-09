#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/AnalyzerData.h"
#include "../AnalyzerViewState.h"
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
    // Screen bounds for the RMS fill
    juce::Rectangle<float> rmsBounds;
    // Y position for the peak cap
    float peakY = 0.0f;
    // Current RMS level for that bar
    float rmsDb = 0.0f;
    // Current peak level for that bar
    float peakDb = 0.0f;
    // Whether this bar is currently hovered
    bool isHovered = false;
};

/**
 * Draw-ready state for one visible analyzer trace
 */
struct AnalyzerTraceVisual {
    // Logical trace identity
    Analyzer::TraceKind kind = Analyzer::TraceKind::input;
    // Draw-ready bar state for this trace
    std::vector<AnalyzerBarModel> bars;
};

/**
 * Builds draw-ready analyzer UI state from the latest composite snapshot
 */
class AnalyzerViewModel final {
public:
    AnalyzerViewModel();

    /**
     * Rebuilds the view model from the latest analyzer snapshot and view state
     */
    void update(const Analyzer::CompositeSnapshot &snapshot, const AnalyzerViewState &viewState,
                float gridMinDb, float gridMaxDb, float gridStepDb,
                const juce::Rectangle<float> &localBounds,
                const std::optional<juce::Point<float>> &hoverPosition);

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
     * Returns the current draw-ready analyzer traces
     */
    const std::vector<AnalyzerTraceVisual> &getTraceVisuals() const;

    /**
     * Returns the current hover tooltip if any
     */
    const std::optional<AnalyzerHoverInfo> &getHoverInfo() const;

    /**
     * Returns the current minimum visible dB
     */
    float getGridMinDb() const;

private:
    void updateGrid(float gridMinDb, float gridMaxDb, float gridStepDb);
    void updateTraceVisuals(const Analyzer::CompositeSnapshot &snapshot, const AnalyzerViewState &viewState,
                            float gridMinDb, float gridMaxDb,
                            const std::optional<juce::Point<float>> &hoverPosition);
    std::optional<Analyzer::TraceSnapshot> getPrimaryVisibleTrace(const Analyzer::CompositeSnapshot &snapshot,
                                                                  const AnalyzerViewState &viewState) const;
    bool isTraceEnabled(Analyzer::TraceKind kind, const AnalyzerViewState &viewState) const;
    static float getRmsDb(size_t bandIndex, const Analyzer::Frame &latestFrame, float gridMinDb);
    static float getPeakDb(size_t bandIndex, const Analyzer::Frame &latestFrame, float gridMinDb);
    void updateVisibleFrequencyRange(const Analyzer::CompositeSnapshot &snapshot, const AnalyzerViewState &viewState);

    AnalyzerGeometry geometry;
    FrequencyFormatter formatter;
    MusicTheory musicTheory;
    AnalyzerHoverModel hoverModel;

    juce::Rectangle<float> plotBounds;
    std::vector<AnalyzerGridLine> gridLines;
    std::vector<AnalyzerFrequencyMarker> frequencyMarkers;
    std::vector<AnalyzerTraceVisual> traceVisuals;
    std::optional<AnalyzerHoverInfo> hoverInfo;
    float currentGridMinDb = 0.0f;
    float visibleMinFrequencyHz = 20.0f;
    float visibleMaxFrequencyHz = 20000.0f;
};
