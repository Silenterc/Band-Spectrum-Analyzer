#include "AnalyzerHoverOverlayComponent.h"

AnalyzerHoverOverlayComponent::AnalyzerHoverOverlayComponent(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
    setOpaque(false);
    setInterceptsMouseClicks(false, false);
    tooltipLineGlyphs.resize(theme.metrics.tooltip.maxLines);
}

void AnalyzerHoverOverlayComponent::paint(juce::Graphics &g) {
    drawHoveredBars(g);
    drawTooltip(g);
}

void AnalyzerHoverOverlayComponent::updateState(const std::optional<AnalyzerHoverInfo> &newHoverInfo,
                                                const std::vector<AnalyzerTraceVisual> &traceVisuals,
                                                const float newGridMinDb,
                                                const std::optional<juce::Rectangle<float>> &newHoveredBandBounds) {
    auto dirtyBounds = getDirtyBounds(hoverInfo, hoveredBandBounds);
    const auto tooltipContentChanged = !hasSameTooltipContent(newHoverInfo);

    hoverInfo = newHoverInfo;
    hoveredBandBounds = newHoveredBandBounds;
    gridMinDb = newGridMinDb;
    rebuildHoveredTraceBars(traceVisuals);
    if (tooltipContentChanged) {
        rebuildTooltipChrome();
        rebuildTooltipGlyphs();
    }

    dirtyBounds = dirtyBounds.getUnion(getDirtyBounds(hoverInfo, hoveredBandBounds));
    if (!dirtyBounds.isEmpty())
        repaint(dirtyBounds);
}

void AnalyzerHoverOverlayComponent::drawHoveredBars(juce::Graphics &g) const {
    if (!hoverInfo.has_value())
        return;

    const auto clipBounds = g.getClipBounds().toFloat();

    for (const auto &traceBar: hoveredTraceBars) {
        const auto &bar = traceBar.bar;
        if (!bar.bandBounds.intersects(clipBounds))
            continue;

        if (bar.peakDb > gridMinDb) {
            g.setColour(traceBar.peakColour);
            const auto peakBounds = bar.peakBounds.getSmallestIntegerContainer();
            if (!peakBounds.isEmpty())
                g.fillRect(peakBounds);
        }

        if (bar.rmsDb > gridMinDb) {
            g.setColour(traceBar.rmsColour);
            const auto rmsBounds = bar.rmsBounds.getSmallestIntegerContainer();
            if (!rmsBounds.isEmpty())
                g.fillRect(rmsBounds);
        }

    }
}

void AnalyzerHoverOverlayComponent::drawTooltip(juce::Graphics &g) const {
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

void AnalyzerHoverOverlayComponent::rebuildHoveredTraceBars(const std::vector<AnalyzerTraceVisual> &traceVisuals) {
    hoveredTraceBars.clear();

    if (!hoverInfo.has_value())
        return;

    hoveredTraceBars.reserve(traceVisuals.size());

    for (const auto &traceVisual: traceVisuals) {
        if (hoverInfo->bandIndex >= traceVisual.bars.size())
            continue;

        HoverTraceBarVisual traceBar;
        traceBar.peakColour = traceVisual.colour.brighter(0.18f);
        traceBar.rmsColour = traceBar.peakColour.withMultipliedAlpha(0.45f);
        traceBar.bar = traceVisual.bars[hoverInfo->bandIndex];
        hoveredTraceBars.push_back(std::move(traceBar));
    }
}

juce::Rectangle<int> AnalyzerHoverOverlayComponent::getDirtyBounds(
    const std::optional<AnalyzerHoverInfo> &hoverInfoToMeasure,
    const std::optional<juce::Rectangle<float>> &bandBoundsToMeasure) const {
    if (!hoverInfoToMeasure.has_value())
        return {};

    auto dirtyBounds = hoverInfoToMeasure->bounds.getSmallestIntegerContainer().expanded(2);
    if (bandBoundsToMeasure.has_value())
        dirtyBounds = dirtyBounds.getUnion(bandBoundsToMeasure->getSmallestIntegerContainer().expanded(2));

    return dirtyBounds;
}

void AnalyzerHoverOverlayComponent::rebuildTooltipChrome() {
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

void AnalyzerHoverOverlayComponent::rebuildTooltipGlyphs() {
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

bool AnalyzerHoverOverlayComponent::hasSameTooltipContent(const std::optional<AnalyzerHoverInfo> &otherHoverInfo) const {
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
