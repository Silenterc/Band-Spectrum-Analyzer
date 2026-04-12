#pragma once

#include <cstddef>

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * One visible analyzer band projected into plot-space from the current zoom range.
 */
struct AnalyzerVisibleBandLayout {
    // Source band index in the full analyzer band layout.
    size_t sourceBandIndex = 0;
    // Continuous hit region spanning the visible part of the band.
    juce::Rectangle<float> hitBounds;
    // Draw bounds with the configured inter-band gap applied.
    juce::Rectangle<float> drawBounds;
};
