#include "AnalyzerGeometry.h"

#include <algorithm>
#include <cmath>

namespace {
    constexpr float tooltipWidth = 132.0f;
    constexpr float tooltipHeight = 72.0f;
    constexpr int interBandGapPixels = 1;
}

juce::Rectangle<float> AnalyzerGeometry::getPlotBounds(const juce::Rectangle<float> &localBounds) const {
    auto bounds = localBounds;
    bounds.removeFromTop(AnalyzerLayout::plotMargins.top);
    bounds.removeFromLeft(AnalyzerLayout::plotMargins.left);
    bounds.removeFromRight(AnalyzerLayout::plotMargins.right);
    bounds.removeFromBottom(AnalyzerLayout::plotMargins.bottom);
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

juce::Rectangle<float> AnalyzerGeometry::getBandDrawBounds(const size_t bandIndex,
                                                           const size_t bandCount,
                                                           const juce::Rectangle<float> &plotBounds) const {
    if (bandCount == 0 || bandIndex >= bandCount)
        return {};

    const auto plotBoundsInt = plotBounds.getSmallestIntegerContainer();
    const auto totalGapWidth = juce::jmax(0, static_cast<int>(bandCount - 1)) * interBandGapPixels;
    const auto availableBarWidth = juce::jmax(0, plotBoundsInt.getWidth() - totalGapWidth);
    const auto bandCountInt = static_cast<int>(bandCount);
    const auto leftEdge = (static_cast<int>(bandIndex) * availableBarWidth) / bandCountInt;
    const auto rightEdge = (static_cast<int>(bandIndex + 1) * availableBarWidth) / bandCountInt;
    const auto x = plotBoundsInt.getX() + leftEdge + static_cast<int>(bandIndex) * interBandGapPixels;
    const auto width = juce::jmax(0, rightEdge - leftEdge);
    return juce::Rectangle<int>(x, plotBoundsInt.getY(), width, plotBoundsInt.getHeight()).toFloat();
}

juce::Rectangle<float> AnalyzerGeometry::getBandHitBounds(const size_t bandIndex,
                                                          const size_t bandCount,
                                                          const juce::Rectangle<float> &plotBounds) const {
    if (bandCount == 0 || bandIndex >= bandCount)
        return {};

    const auto plotBoundsInt = plotBounds.getSmallestIntegerContainer();
    const auto bandCountInt = static_cast<int>(bandCount);
    const auto leftEdge = (static_cast<int>(bandIndex) * plotBoundsInt.getWidth()) / bandCountInt;
    const auto rightEdge = (static_cast<int>(bandIndex + 1) * plotBoundsInt.getWidth()) / bandCountInt;
    const auto width = juce::jmax(0, rightEdge - leftEdge);
    return juce::Rectangle<int>(plotBoundsInt.getX() + leftEdge,
                                plotBoundsInt.getY(),
                                width,
                                plotBoundsInt.getHeight()).toFloat();
}

std::optional<size_t> AnalyzerGeometry::bandIndexAt(juce::Point<float> position, size_t bandCount,
                                                    const juce::Rectangle<float> &plotBounds) const {
    if (bandCount == 0 || !plotBounds.contains(position))
        return std::nullopt;

    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        if (getBandHitBounds(bandIndex, bandCount, plotBounds).contains(position))
            return bandIndex;
    }

    return std::nullopt;
}

juce::Rectangle<float> AnalyzerGeometry::getBarBounds(size_t bandIndex, size_t bandCount, float displayedDb, float minDb,
                                                      float maxDb, const juce::Rectangle<float> &plotBounds) const {
    if (bandCount == 0)
        return {};

    const auto bandDrawBounds = getBandDrawBounds(bandIndex, bandCount, plotBounds);
    if (bandDrawBounds.isEmpty())
        return {};

    const auto y = yForDb(displayedDb, minDb, maxDb, plotBounds);
    return {bandDrawBounds.getX(), y, bandDrawBounds.getWidth(), plotBounds.getBottom() - y};
}

juce::Rectangle<float> AnalyzerGeometry::getTooltipBounds(juce::Point<float> hoverPosition,
                                                          const juce::Rectangle<float> &plotBounds,
                                                          const juce::Rectangle<float> &localBounds) const {
    // Keep the box slightly below the cursor so the pointer does not sit on its vertical centre
    auto tooltipBounds = juce::Rectangle<float>(hoverPosition.x + 12.0f, hoverPosition.y + 10.0f,
                                                tooltipWidth, tooltipHeight);

    if (tooltipBounds.getRight() > localBounds.getRight() - 8.0f)
        tooltipBounds.setX(hoverPosition.x - tooltipWidth - 12.0f);

    if (tooltipBounds.getY() < plotBounds.getY())
        tooltipBounds.setY(plotBounds.getY());

    if (tooltipBounds.getBottom() > localBounds.getBottom() - 8.0f)
        tooltipBounds.setY(localBounds.getBottom() - tooltipHeight - 8.0f);

    return tooltipBounds;
}
