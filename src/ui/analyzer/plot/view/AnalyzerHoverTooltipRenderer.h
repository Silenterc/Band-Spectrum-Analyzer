#pragma once

#include <optional>
#include <vector>

#include <juce_graphics/juce_graphics.h>

#include "ui/theme/UiTheme.h"
#include "ui/analyzer/plot/logic/AnalyzerHoverModel.h"

class AnalyzerHoverTooltipRenderer final {
public:
    explicit AnalyzerHoverTooltipRenderer(const Ui::Theme &theme);

    void draw(juce::Graphics &g) const;

    [[nodiscard]] juce::Rectangle<int> updateState(const std::optional<AnalyzerHoverInfo> &newHoverInfo);

private:
    void rebuildTooltipChrome();
    void rebuildTooltipGlyphs();
    bool hasSameTooltipContent(const std::optional<AnalyzerHoverInfo> &otherHoverInfo) const;
    juce::Rectangle<int> getDirtyBounds(const std::optional<AnalyzerHoverInfo> &hoverInfoToMeasure) const;

    const Ui::Theme &theme;
    std::optional<AnalyzerHoverInfo> hoverInfo;
    juce::Image tooltipChromeImage;
    std::vector<juce::GlyphArrangement> tooltipLineGlyphs;
};
