#pragma once

#include <optional>
#include <vector>

#include <juce_graphics/juce_graphics.h>

#include "display/analyzer/data/AnalyzerDisplayFrame.h"
#include "display/analyzer/data/AnalyzerMeterData.h"
#include "shared/SignalSlotConfiguration.h"
#include "ui/theme/UiTheme.h"
#include "ui/analyzer/plot/logic/AnalyzerRenderBatchBuilder.h"
#include "ui/analyzer/plot/logic/AnalyzerViewModel.h"

class AnalyzerHoverOverlayRenderer final {
public:
    explicit AnalyzerHoverOverlayRenderer(const Ui::Theme &theme);

    void draw(juce::Graphics &g) const;

    [[nodiscard]] juce::Rectangle<int> updateState(
        const std::optional<AnalyzerHoverInfo> &newHoverInfo,
        const AnalyzerDisplayFrame *displayFrame,
        const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
        const Shared::SignalSlotOrder &signalSlotOrder,
        std::uint64_t highlightVisualRevision,
        const Analyzer::MeterSettings &meterSettings,
        float gridMinDb,
        float gridMaxDb,
        const juce::Rectangle<float> &plotBounds,
        const std::optional<juce::Rectangle<float>> &hoveredBandBounds);

private:
    void drawHoveredBars(juce::Graphics &g) const;
    void drawTooltip(juce::Graphics &g) const;
    void rebuildTooltipChrome();
    void rebuildTooltipGlyphs();
    bool hasSameTooltipContent(const std::optional<AnalyzerHoverInfo> &otherHoverInfo) const;
    juce::Rectangle<int> getDirtyBounds(const std::optional<AnalyzerHoverInfo> &hoverInfoToMeasure,
                                        const std::optional<juce::Rectangle<float>> &bandBoundsToMeasure) const;

    const Ui::Theme &theme;
    std::optional<AnalyzerHoverInfo> hoverInfo;
    std::optional<juce::Rectangle<float>> hoveredBandBounds;
    juce::Image tooltipChromeImage;
    std::vector<juce::GlyphArrangement> tooltipLineGlyphs;
    AnalyzerRenderBatchBuilder renderBatchBuilder;
    std::optional<size_t> highlightedBandIndex;
    std::uint64_t highlightedVisualRevision = 0;
    float gridMinDb = 0.0f;
    float gridMaxDb = 0.0f;
};
