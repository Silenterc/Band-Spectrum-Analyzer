#pragma once

#include <vector>

#include <juce_graphics/juce_graphics.h>

#include "display/analyzer/data/AnalyzerDisplayFrame.h"
#include "display/analyzer/data/AnalyzerMeterData.h"
#include "ui/analyzer/plot/logic/AnalyzerVisibleBandLayout.h"
#include "ui/state/SignalSlotUiState.h"
#include "ui/theme/UiTheme.h"

class AnalyzerRenderBatchBuilder final {
public:
    struct Batch {
        juce::Colour colour;
        juce::RectangleList<int> rectangles;
    };

    struct PathBatch {
        juce::Colour colour;
        juce::Path path;
        float strokeThickness = 1.0f;
    };

    explicit AnalyzerRenderBatchBuilder(const Ui::Theme &themeToUse)
        : theme(themeToUse) {
    }

    void reset();

    void buildTraceBatches(const AnalyzerDisplayFrame *displayFrame,
                           const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                           const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                           const Shared::SignalSlotOrder &signalSlotOrder,
                           const Analyzer::MeterSettings &meterSettings,
                           float gridMinDb,
                           float gridMaxDb,
                           const juce::Rectangle<float> &plotBounds,
                           const juce::Rectangle<float> &clipBounds);

    void buildGlobalHoldBatches(const AnalyzerDisplayFrame *displayFrame,
                                const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                                const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                const Analyzer::MeterSettings &meterSettings,
                                float gridMinDb,
                                float gridMaxDb,
                                const juce::Rectangle<float> &plotBounds,
                                const juce::Rectangle<float> &clipBounds);

    void buildHoveredBarBatches(const AnalyzerDisplayFrame *displayFrame,
                                const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                                const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                const Shared::SignalSlotOrder &signalSlotOrder,
                                const Analyzer::MeterSettings &meterSettings,
                                size_t hoveredBandIndex,
                                float gridMinDb,
                                float gridMaxDb,
                                const juce::Rectangle<float> &plotBounds,
                                const juce::Rectangle<float> &clipBounds);

    const std::vector<Batch> &getBatches() const;
    const std::vector<PathBatch> &getPathBatches() const;

private:
    static float yForDb(float decibels, float minDb, float maxDb, const juce::Rectangle<float> &plotBounds);
    static float getBandValue(const std::vector<float> &values, size_t bandIndex, float floorDb);
    static juce::Colour getTraceColour(const Ui::SignalSlotState &slot);
    juce::Colour makeGlobalHoldColour(const std::optional<Analyzer::TraceKind> &ownerKind,
                                      const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots) const;
    void addRect(const juce::Colour &colour, const juce::Rectangle<int> &bounds);
    void addPath(const juce::Colour &colour, const juce::Path &path, float strokeThickness);

    const Ui::Theme &theme;
    std::vector<Batch> batches;
    std::vector<PathBatch> pathBatches;
};
