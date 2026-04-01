#include "AnalyzerGeometry.h"

#include <algorithm>
#include <cmath>

namespace {
}

juce::Rectangle<float> AnalyzerGeometry::getPlotBounds(const juce::Rectangle<float> &localBounds) const {
    auto bounds = localBounds;
    const auto &plotMetrics = theme.metrics.analyzerPlot;
    bounds.removeFromTop(plotMetrics.plotMarginTop);
    bounds.removeFromLeft(plotMetrics.plotMarginLeft);
    bounds.removeFromRight(plotMetrics.plotMarginRight);
    bounds.removeFromBottom(plotMetrics.plotMarginBottom);
    return bounds;
}

float AnalyzerGeometry::xForFrequency(float frequencyHz, float minFrequencyHz, float maxFrequencyHz,
                                      const juce::Rectangle<float> &plotBounds) const {
    if (minFrequencyHz <= 0.0f || maxFrequencyHz <= minFrequencyHz)
        return plotBounds.getX();

    // We draw frequency on a log axis, so we convert Hz -> log10(Hz) before mapping to pixels
    const auto logMin = std::log10(minFrequencyHz);
    const auto logMax = std::log10(maxFrequencyHz);
    const auto logFrequency = std::log10(juce::jlimit(minFrequencyHz, maxFrequencyHz, frequencyHz));
    const auto normalised = (logFrequency - logMin) / (logMax - logMin);
    return plotBounds.getX() + normalised * plotBounds.getWidth();
}

float AnalyzerGeometry::frequencyForX(float x, float minFrequencyHz, float maxFrequencyHz,
                                      const juce::Rectangle<float> &plotBounds) const {
    if (minFrequencyHz <= 0.0f || maxFrequencyHz <= minFrequencyHz)
        return 0.0f;

    const auto width = std::max(plotBounds.getWidth(), 1.0f);
    const auto normalised = juce::jlimit(0.0f, 1.0f, (x - plotBounds.getX()) / width);
    // This is the inverse of xForFrequency: pixel -> normalised -> log10(Hz) -> Hz
    const auto logMin = std::log10(minFrequencyHz);
    const auto logMax = std::log10(maxFrequencyHz);
    const auto logFrequency = juce::jmap(normalised, logMin, logMax);
    return std::pow(10.0f, logFrequency);
}

float AnalyzerGeometry::yForDb(float decibels, float minDb, float maxDb,
                               const juce::Rectangle<float> &plotBounds) const {
    const auto clampedDb = juce::jlimit(minDb, maxDb, decibels);
    const auto normalised = juce::jmap(clampedDb, minDb, maxDb, 0.0f, 1.0f);
    return plotBounds.getBottom() - normalised * plotBounds.getHeight();
}

float AnalyzerGeometry::dbForY(float y, float minDb, float maxDb,
                               const juce::Rectangle<float> &plotBounds) const {
    if (maxDb <= minDb || plotBounds.getHeight() <= 0.0f)
        return minDb;

    const auto clampedY = juce::jlimit(plotBounds.getY(), plotBounds.getBottom(), y);
    const auto normalised = juce::jlimit(0.0f, 1.0f, (plotBounds.getBottom() - clampedY) / plotBounds.getHeight());
    return juce::jmap(normalised, 0.0f, 1.0f, minDb, maxDb);
}

juce::Rectangle<float> AnalyzerGeometry::getBandHitBounds(const float lowFrequencyHz,
                                                          const float highFrequencyHz,
                                                          const float visibleMinFrequencyHz,
                                                          const float visibleMaxFrequencyHz,
                                                          const juce::Rectangle<float> &plotBounds) const {
    if (visibleMinFrequencyHz <= 0.0f || visibleMaxFrequencyHz <= visibleMinFrequencyHz)
        return {};

    const auto clampedLowHz = juce::jlimit(visibleMinFrequencyHz, visibleMaxFrequencyHz, lowFrequencyHz);
    const auto clampedHighHz = juce::jlimit(visibleMinFrequencyHz, visibleMaxFrequencyHz, highFrequencyHz);
    if (clampedHighHz <= clampedLowHz)
        return {};

    const auto leftX = xForFrequency(clampedLowHz, visibleMinFrequencyHz, visibleMaxFrequencyHz, plotBounds);
    const auto rightX = xForFrequency(clampedHighHz, visibleMinFrequencyHz, visibleMaxFrequencyHz, plotBounds);
    if (rightX <= leftX)
        return {};

    return {leftX, plotBounds.getY(), rightX - leftX, plotBounds.getHeight()};
}

juce::Rectangle<float> AnalyzerGeometry::getTooltipBounds(juce::Point<float> hoverPosition,
                                                          const juce::Rectangle<float> &plotBounds,
                                                          const juce::Rectangle<float> &localBounds) const {
    // Keep the box slightly below the cursor so the pointer does not sit on its vertical centre
    const auto &tooltip = theme.metrics.tooltip;
    auto tooltipBounds = juce::Rectangle<float>(hoverPosition.x + tooltip.offsetX,
                                                hoverPosition.y + tooltip.offsetY,
                                                tooltip.width,
                                                tooltip.height);

    if (tooltipBounds.getRight() > localBounds.getRight() - tooltip.edgeInset)
        tooltipBounds.setX(hoverPosition.x - tooltip.width - tooltip.offsetX);

    if (tooltipBounds.getY() < plotBounds.getY())
        tooltipBounds.setY(plotBounds.getY());

    if (tooltipBounds.getBottom() > localBounds.getBottom() - tooltip.edgeInset)
        tooltipBounds.setY(localBounds.getBottom() - tooltip.height - tooltip.edgeInset);

    return tooltipBounds;
}
