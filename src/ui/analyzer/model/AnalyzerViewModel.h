#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/core/AnalyzerData.h"
#include "../AnalyzerUiConstants.h"
#include "../../SignalSlotUiState.h"
#include "../../UiTheme.h"
#include "../AnalyzerViewState.h"
#include "../AnalyzerRenderData.h"
#include "../helpers/AnalyzerGeometry.h"
#include "../helpers/AnalyzerHoverModel.h"
#include "../helpers/FrequencyFormatter.h"
#include "../helpers/MusicTheory.h"
#include "AnalyzerUiSelectors.h"
#include "SignalSlotOrderModel.h"

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
    // Full repaint bounds for this band's column inside the plot
    juce::Rectangle<float> bandBounds;
    // Screen bounds for the full peak bar
    juce::Rectangle<float> peakBounds;
    // Screen bounds for the RMS section inside the peak bar
    juce::Rectangle<float> rmsBounds;
    // Y position for the peak cap
    float peakY = 0.0f;
    // Current RMS level for that bar
    float rmsDb = 0.0f;
    // Current peak level for that bar
    float peakDb = 0.0f;
};

/**
 * Draw-ready state for one visible analyzer trace
 */
struct AnalyzerTraceVisual {
    // Logical trace identity
    Analyzer::TraceKind kind = Analyzer::TraceKind::slot1;
    // Solid display colour for this trace
    juce::Colour colour;
    // Draw-ready bar state for this trace
    std::vector<AnalyzerBarModel> bars;
};

/**
 * Builds draw-ready analyzer UI state from the latest meter data
 */
class AnalyzerViewModel final {
public:
    explicit AnalyzerViewModel(const Ui::Theme &themeToUse);

    /**
     * Rebuilds the static analyzer layout that only changes when bounds or scale change
     */
    void updateStaticLayout(const Analyzer::RenderData &renderData, const AnalyzerViewState &viewState,
                            float gridMinDb, float gridMaxDb, float gridStepDb,
                            const juce::Rectangle<float> &localBounds);

    /**
     * Rebuilds dynamic per-trace bar geometry from the latest meter data
     */
    void updateTraceVisuals(const Analyzer::RenderData &renderData, const AnalyzerViewState &viewState,
                            const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                            const Shared::SignalSlotOrder &signalSlotOrder,
                            float gridMinDb, float gridMaxDb);

    /**
     * Updates hover-only state without rebuilding static or dynamic bar geometry
     */
    void updateHover(const Analyzer::RenderData &renderData,
                     float gridMinDb,
                     float gridMaxDb,
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

    /**
     * Returns repaint bounds for one analyzer band column
     */
    std::optional<juce::Rectangle<float>> getBandBounds(size_t bandIndex) const;

private:
    void updateGrid(float gridMinDb, float gridMaxDb, float gridStepDb);
    void updateBandBounds(size_t bandCount);
    static float getRmsDb(size_t bandIndex, const Analyzer::RenderFrame &renderFrame, float gridMinDb);
    static float getPeakDb(size_t bandIndex, const Analyzer::RenderFrame &renderFrame, float gridMinDb);
    void updateVisibleFrequencyRange(const Analyzer::RenderData &renderData, const AnalyzerViewState &viewState);

    const Ui::Theme &theme;
    AnalyzerGeometry geometry;
    FrequencyFormatter formatter;
    MusicTheory musicTheory;
    AnalyzerHoverModel hoverModel;
    SignalSlotOrderModel slotOrderModel;

    juce::Rectangle<float> plotBounds;
    std::vector<AnalyzerGridLine> gridLines;
    std::vector<AnalyzerFrequencyMarker> frequencyMarkers;
    std::vector<juce::Rectangle<float>> bandBounds;
    std::vector<AnalyzerTraceVisual> traceVisuals;
    std::optional<AnalyzerHoverInfo> hoverInfo;
    float currentGridMinDb = 0.0f;
    float visibleMinFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMinFrequencyHz;
    float visibleMaxFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMaxFrequencyHz;
};
