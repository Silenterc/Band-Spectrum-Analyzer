#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "dsp/core/AnalyzerData.h"
#include "ui/analyzer/plot/data/AnalyzerUiConstants.h"
#include "ui/theme/UiTheme.h"
#include "ui/analyzer/plot/data/AnalyzerViewState.h"
#include "ui/analyzer/plot/logic/AnalyzerGeometry.h"
#include "ui/analyzer/plot/logic/AnalyzerHoverModel.h"
#include "ui/analyzer/plot/logic/FrequencyFormatter.h"
#include "ui/analyzer/plot/logic/MusicTheory.h"
#include "AnalyzerVisibleBandLayout.h"

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
 * Builds static analyzer UI state and hover readouts on the message thread
 */
class AnalyzerViewModel final {
public:
    explicit AnalyzerViewModel(const Ui::Theme &themeToUse);

    /**
     * Rebuilds the static analyzer layout that only changes when bounds or scale change
     */
    void updateStaticLayout(const std::vector<Analyzer::BandInfo> &bandInfo, const AnalyzerViewState &viewState,
                            float gridMinDb, float gridMaxDb, float gridStepDb,
                            const juce::Rectangle<float> &localBounds);

    /**
     * Updates hover-only state without rebuilding static or dynamic bar geometry
     */
    void updateHover(float gridMinDb,
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
     * Returns the current hover tooltip if any
     */
    const std::optional<AnalyzerHoverInfo> &getHoverInfo() const;

    /**
     * Returns the current minimum visible dB
     */
    float getGridMinDb() const;

    /**
     * Returns the current visible analyzer band layout
     */
    const std::vector<AnalyzerVisibleBandLayout> &getVisibleBands() const;

private:
    void updateGrid(float gridMinDb, float gridMaxDb, float gridStepDb);
    void updateVisibleBands(const std::vector<Analyzer::BandInfo> &bandInfo);
    void updateVisibleFrequencyRange(const std::vector<Analyzer::BandInfo> &bandInfo,
                                     const AnalyzerViewState &viewState);

    const Ui::Theme &theme;
    AnalyzerGeometry geometry;
    FrequencyFormatter formatter;
    MusicTheory musicTheory;
    AnalyzerHoverModel hoverModel;

    juce::Rectangle<float> plotBounds;
    std::vector<AnalyzerGridLine> gridLines;
    std::vector<AnalyzerFrequencyMarker> frequencyMarkers;
    std::vector<AnalyzerVisibleBandLayout> visibleBands;
    std::optional<AnalyzerHoverInfo> hoverInfo;
    float currentGridMinDb = 0.0f;
    bool usingCustomFrequencyRange = false;
    float visibleMinFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMinFrequencyHz;
    float visibleMaxFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMaxFrequencyHz;
};
