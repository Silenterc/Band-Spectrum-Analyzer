#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../AnalyzerDataSource.h"
#include "../UiTheme.h"
#include "AnalyzerRenderData.h"
#include "AnalyzerViewState.h"
#include "helpers/AnalyzerMeter.h"
#include "helpers/AnalyzerViewModel.h"

/**
 * Draws the analyzer plot from a precomputed view model
 */
class AnalyzerComponent final : public juce::Component, private juce::Timer {
public:
    /**
     * Binds the component to a read-only analyzer data source
     */
    AnalyzerComponent(AnalyzerDataSource &dataSource, const Ui::Theme &theme);

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;

private:
    /**
     * On a freeze edge, restores the snapshot the user actually saw on screen
     */
    void syncFreezeSnapshotIfNeeded();

    /**
     * Draws the dB grid and fixed frequency labels
     */
    void drawGrid(juce::Graphics &g) const;

    /**
     * Draws one vertical bar per analyzer band
     */
    void drawBars(juce::Graphics &g) const;

    /**
     * Draws the hover tooltip with dB, frequency, and note
     */
    void drawHoverInfo(juce::Graphics &g) const;

    /**
     * Rebuilds the view model from the latest snapshot and hover state
     */
    void rebuildViewModel();

    /**
     * Pulls the latest published snapshot and repaints
     */
    void timerCallback() override;

    // Read-only analyzer data source
    AnalyzerDataSource &dataSource;
    // Shared UI theme
    const Ui::Theme &theme;
    // Latest immutable band layout from the processor
    std::shared_ptr<const std::vector<Analyzer::BandInfo>> bandInfo;
    // Latest raw traces from the processor
    std::vector<Analyzer::RawTrace> rawTraces;
    // Display-rate meter processor
    AnalyzerMeter displayMeter;
    // Latest meter-processed data
    Analyzer::RenderData renderData;
    // Last render data that was actually painted to screen
    Analyzer::RenderData lastPaintedRenderData;
    // Current analyzer draw model
    AnalyzerViewModel viewModel;
    // UI-only presentation state such as visible trace set and zoom
    AnalyzerViewState viewState;
    // Raw mouse position used by the hover model
    std::optional<juce::Point<float>> hoverPosition;
    // Last timer timestamp used to compute dt
    double lastPollTimeMs = 0.0;
    // Tracks freeze transitions on the message thread
    bool wasFrozen = false;
};
