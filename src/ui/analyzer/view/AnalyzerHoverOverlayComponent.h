#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../UiTheme.h"
#include "../model/AnalyzerViewModel.h"

class AnalyzerHoverOverlayComponent final : public juce::Component {
public:
    explicit AnalyzerHoverOverlayComponent(const Ui::Theme &theme);

    void paint(juce::Graphics &g) override;

    void updateState(const std::optional<AnalyzerHoverInfo> &newHoverInfo,
                     const std::vector<AnalyzerTraceVisual> &traceVisuals,
                     float gridMinDb,
                     const std::optional<juce::Rectangle<float>> &hoveredBandBounds);

private:
    struct HoverTraceBarVisual {
        juce::Colour peakColour;
        juce::Colour rmsColour;
        juce::Colour lineColour;
        AnalyzerBarModel bar;
    };

    void drawHoveredBars(juce::Graphics &g) const;
    void drawTooltip(juce::Graphics &g) const;
    void rebuildHoveredTraceBars(const std::vector<AnalyzerTraceVisual> &traceVisuals);
    void rebuildTooltipChrome();
    void rebuildTooltipGlyphs();
    bool hasSameTooltipContent(const std::optional<AnalyzerHoverInfo> &otherHoverInfo) const;
    juce::Rectangle<int> getDirtyBounds(const std::optional<AnalyzerHoverInfo> &hoverInfoToMeasure,
                                        const std::optional<juce::Rectangle<float>> &bandBoundsToMeasure) const;

    const Ui::Theme &theme;
    std::optional<AnalyzerHoverInfo> hoverInfo;
    std::optional<juce::Rectangle<float>> hoveredBandBounds;
    std::vector<HoverTraceBarVisual> hoveredTraceBars;
    juce::Image tooltipChromeImage;
    std::vector<juce::GlyphArrangement> tooltipLineGlyphs;
    float gridMinDb = 0.0f;
};
