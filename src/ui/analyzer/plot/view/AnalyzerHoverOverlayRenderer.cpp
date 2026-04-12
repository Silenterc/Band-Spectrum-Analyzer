#include "ui/analyzer/plot/view/AnalyzerHoverOverlayRenderer.h"

AnalyzerHoverOverlayRenderer::AnalyzerHoverOverlayRenderer(const Ui::Theme &themeToUse)
    : theme(themeToUse),
      renderBatchBuilder(themeToUse) {
    tooltipLineGlyphs.resize(theme.metrics.tooltip.maxLines);
}

void AnalyzerHoverOverlayRenderer::draw(juce::Graphics &g) const {
    drawHoveredBars(g);
    drawTooltip(g);
}

juce::Rectangle<int> AnalyzerHoverOverlayRenderer::updateState(
    const std::optional<AnalyzerHoverInfo> &newHoverInfo,
    const AnalyzerDisplayFrame *displayFrame,
    const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
    const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
    const Shared::SignalSlotOrder &signalSlotOrder,
    const std::uint64_t highlightVisualRevision,
    const Analyzer::MeterSettings &meterSettings,
    const float newGridMinDb,
    const float newGridMaxDb,
    const juce::Rectangle<float> &plotBounds,
    const std::optional<juce::Rectangle<float>> &newHoveredBandBounds) {
    auto dirtyBounds = getDirtyBounds(hoverInfo, hoveredBandBounds);
    const auto tooltipContentChanged = !hasSameTooltipContent(newHoverInfo);

    hoverInfo = newHoverInfo;
    hoveredBandBounds = newHoveredBandBounds;
    gridMinDb = newGridMinDb;
    gridMaxDb = newGridMaxDb;
    if (hoverInfo.has_value() && hoveredBandBounds.has_value()) {
        const auto shouldRebuildHighlight = !highlightedBandIndex.has_value()
                                            || *highlightedBandIndex != hoverInfo->bandIndex
                                            || highlightedVisualRevision != highlightVisualRevision;
        if (shouldRebuildHighlight) {
            renderBatchBuilder.buildHoveredBarBatches(displayFrame,
                                                      visibleBands,
                                                      signalSlots,
                                                      signalSlotOrder,
                                                      meterSettings,
                                                      hoverInfo->bandIndex,
                                                      gridMinDb,
                                                      gridMaxDb,
                                                      plotBounds,
                                                      *hoveredBandBounds);
            highlightedBandIndex = hoverInfo->bandIndex;
            highlightedVisualRevision = highlightVisualRevision;
        }
    } else {
        renderBatchBuilder.reset();
        highlightedBandIndex.reset();
        highlightedVisualRevision = 0;
    }
    if (tooltipContentChanged) {
        rebuildTooltipChrome();
        rebuildTooltipGlyphs();
    }

    dirtyBounds = dirtyBounds.getUnion(getDirtyBounds(hoverInfo, hoveredBandBounds));
    return dirtyBounds;
}

void AnalyzerHoverOverlayRenderer::drawHoveredBars(juce::Graphics &g) const {
    if (!hoverInfo.has_value())
        return;

    for (const auto &batch: renderBatchBuilder.getBatches()) {
        if (batch.rectangles.isEmpty())
            continue;

        g.setColour(batch.colour);
        g.fillRectList(batch.rectangles);
    }
}

void AnalyzerHoverOverlayRenderer::drawTooltip(juce::Graphics &g) const {
    if (!hoverInfo.has_value() || !tooltipChromeImage.isValid())
        return;

    const auto tooltipBounds = hoverInfo->bounds.toNearestInt();
    g.drawImageAt(tooltipChromeImage, tooltipBounds.getX(), tooltipBounds.getY());

    juce::Graphics::ScopedSaveState saveState(g);
    g.addTransform(juce::AffineTransform::translation(static_cast<float>(tooltipBounds.getX()),
                                                      static_cast<float>(tooltipBounds.getY())));
    g.setColour(theme.tooltipText);

    for (size_t lineIndex = 0; lineIndex < hoverInfo->lineCount; ++lineIndex)
        tooltipLineGlyphs[lineIndex].draw(g);
}

juce::Rectangle<int> AnalyzerHoverOverlayRenderer::getDirtyBounds(
    const std::optional<AnalyzerHoverInfo> &hoverInfoToMeasure,
    const std::optional<juce::Rectangle<float>> &bandBoundsToMeasure) const {
    if (!hoverInfoToMeasure.has_value())
        return {};

    auto dirtyBounds = hoverInfoToMeasure->bounds.getSmallestIntegerContainer().expanded(2);
    if (bandBoundsToMeasure.has_value())
        dirtyBounds = dirtyBounds.getUnion(bandBoundsToMeasure->getSmallestIntegerContainer().expanded(2));

    return dirtyBounds;
}

void AnalyzerHoverOverlayRenderer::rebuildTooltipChrome() {
    if (!hoverInfo.has_value()) {
        tooltipChromeImage = {};
        return;
    }

    const auto tooltipBounds = hoverInfo->bounds.toNearestInt();
    if (tooltipBounds.isEmpty()) {
        tooltipChromeImage = {};
        return;
    }

    tooltipChromeImage = juce::Image(juce::Image::ARGB, tooltipBounds.getWidth(), tooltipBounds.getHeight(), true);
    juce::Graphics g(tooltipChromeImage);
    const auto localBounds = juce::Rectangle<float>(0.0f, 0.0f,
                                                    static_cast<float>(tooltipBounds.getWidth()),
                                                    static_cast<float>(tooltipBounds.getHeight()));
    const auto &tooltipMetrics = theme.metrics.tooltip;

    juce::ColourGradient fillGradient(
        theme.tooltipBackground.brighter(tooltipMetrics.fillTopBrightness),
        localBounds.getCentreX(),
        localBounds.getY(),
        theme.tooltipBackground.darker(tooltipMetrics.fillBottomDarkness),
        localBounds.getCentreX(),
        localBounds.getBottom(),
        false);
    fillGradient.addColour(tooltipMetrics.fillMidPoint, theme.tooltipBackground);
    g.setGradientFill(fillGradient);
    g.fillRoundedRectangle(localBounds, tooltipMetrics.cornerRadius);

    const auto highlightBounds = localBounds.reduced(1.0f, 1.0f);
    juce::ColourGradient highlightGradient(
        juce::Colours::white.withAlpha(tooltipMetrics.highlightStartAlpha),
        highlightBounds.getCentreX(),
        highlightBounds.getY(),
        juce::Colours::white.withAlpha(tooltipMetrics.highlightEndAlpha),
        highlightBounds.getCentreX(),
        highlightBounds.getY() + highlightBounds.getHeight() * tooltipMetrics.highlightHeightFraction,
        false);
    g.setGradientFill(highlightGradient);
    g.fillRoundedRectangle(highlightBounds, tooltipMetrics.cornerRadius - 1.0f);

    g.setColour(theme.tooltipBorder);
    g.drawRoundedRectangle(localBounds, tooltipMetrics.cornerRadius, 1.0f);
}

void AnalyzerHoverOverlayRenderer::rebuildTooltipGlyphs() {
    for (auto &glyphs: tooltipLineGlyphs)
        glyphs.clear();

    if (!hoverInfo.has_value())
        return;

    const auto tooltipBounds = hoverInfo->bounds.toNearestInt();
    if (tooltipBounds.isEmpty())
        return;

    const auto &tooltipMetrics = theme.metrics.tooltip;
    const auto textBounds = tooltipBounds.withPosition(0, 0).reduced(tooltipMetrics.textPaddingX, tooltipMetrics.textPaddingY);
    const auto lineHeight = tooltipMetrics.lineHeight;
    const juce::Font tooltipFont(juce::FontOptions{}.withHeight(tooltipMetrics.fontHeight));

    for (size_t lineIndex = 0; lineIndex < hoverInfo->lineCount; ++lineIndex) {
        const auto y = textBounds.getY() + static_cast<int>(lineIndex) * lineHeight;
        tooltipLineGlyphs[lineIndex].addFittedText(tooltipFont,
                                                   hoverInfo->lines[lineIndex],
                                                   static_cast<float>(textBounds.getX()),
                                                   static_cast<float>(y),
                                                   static_cast<float>(textBounds.getWidth()),
                                                   static_cast<float>(lineHeight),
                                                   juce::Justification::centredLeft,
                                                   1);
    }
}

bool AnalyzerHoverOverlayRenderer::hasSameTooltipContent(
    const std::optional<AnalyzerHoverInfo> &otherHoverInfo) const {
    if (hoverInfo.has_value() != otherHoverInfo.has_value())
        return false;

    if (!hoverInfo.has_value())
        return true;

    const auto currentBounds = hoverInfo->bounds.toNearestInt();
    const auto nextBounds = otherHoverInfo->bounds.toNearestInt();
    if (currentBounds.getWidth() != nextBounds.getWidth() || currentBounds.getHeight() != nextBounds.getHeight())
        return false;

    if (hoverInfo->lineCount != otherHoverInfo->lineCount)
        return false;

    for (size_t lineIndex = 0; lineIndex < hoverInfo->lineCount; ++lineIndex) {
        if (hoverInfo->lines[lineIndex] != otherHoverInfo->lines[lineIndex])
            return false;
    }

    return true;
}
