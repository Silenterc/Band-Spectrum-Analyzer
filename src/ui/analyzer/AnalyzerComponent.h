#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../shared/SignalSlotConfiguration.h"
#include "../AnalyzerDataSource.h"
#include "../SignalSlotUiState.h"
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
    struct StaticViewStateKey {
        juce::Rectangle<int> bounds;
        size_t bandCount = 0;
        float minBandFrequencyHz = 0.0f;
        float maxBandFrequencyHz = 0.0f;
        float gridMinDb = 0.0f;
        float gridMaxDb = 0.0f;
        float gridStepDb = 0.0f;
        bool useCustomFrequencyRange = false;
        float visibleMinFrequencyHz = 0.0f;
        float visibleMaxFrequencyHz = 0.0f;

        bool operator==(const StaticViewStateKey &other) const;
    };

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
     * Refreshes the latest UI-state snapshot from the processor bridge
     */
    bool refreshUiSnapshot();

    /**
     * Rebuilds the enabled-trace view state from the stored slot snapshot
     */
    void rebuildEnabledTraces();

    /**
     * Updates cached static layout state when geometry or scale inputs change
     */
    void refreshStaticViewModelIfNeeded();

    /**
     * Rebuilds dynamic bar geometry from the latest render data
     */
    void rebuildDynamicViewModel();

    /**
     * Updates hover state without rebuilding static layout or bars
     */
    void updateHoverState();

    /**
     * Renders the static analyzer layer into its backing image when dirty
     */
    void ensureStaticLayer();

    /**
     * Repaints only the old/new hover highlight and tooltip regions
     */
    void repaintHoverDelta(const std::optional<AnalyzerHoverInfo> &previousHoverInfo);

    /**
     * Returns the repaint bounds for one hover state
     */
    juce::Rectangle<int> getHoverDirtyBounds(const std::optional<AnalyzerHoverInfo> &hoverInfo) const;

    /**
     * Adjusts the polling interval for active vs idle analyzer states
     */
    void setIdlePolling(bool shouldUseIdlePolling);

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
    // Latest slot presentation state used by the analyzer view model
    std::array<Ui::SignalSlotState, Shared::maxSignalSlots> signalSlots{};
    // Latest UI-configured slot order
    Shared::SignalSlotOrder signalSlotOrder{};
    // Latest UI-configured meter settings
    Analyzer::MeterSettings meterSettings;
    // Latest UI-configured grid scale
    float gridMinDb = 0.0f;
    float gridMaxDb = 0.0f;
    float gridStepDb = 0.0f;
    // Raw mouse position used by the hover model
    std::optional<juce::Point<float>> hoverPosition;
    // Cached static plot background, border, and grid
    juce::Image staticLayer;
    // Whether the static layer must be regenerated before painting
    bool staticLayerDirty = true;
    // Last input set that produced the current static layout
    std::optional<StaticViewStateKey> staticViewStateKey;
    // Last timer timestamp used to compute dt
    double lastPollTimeMs = 0.0;
    // Tracks freeze transitions on the message thread
    bool wasFrozen = false;
    // Tracks whether the analyzer is currently on its slower idle polling cadence
    bool isIdlePolling = false;
};
