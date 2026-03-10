#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/AnalyzerData.h"

/**
 * Converts analyzer data between domain space and screen space
 */
class AnalyzerGeometry final {
public:
    /**
     * Returns the drawable plot area after reserving margins for labels
     */
    juce::Rectangle<float> getPlotBounds(const juce::Rectangle<float> &localBounds) const;

    /**
     * Maps a frequency in Hz onto the plot's log-frequency x axis
     */
    float xForFrequency(float frequencyHz, float minFrequencyHz, float maxFrequencyHz,
                        const juce::Rectangle<float> &plotBounds) const;

    /**
     * Maps a cursor x position back into a frequency on the same log axis
     */
    float frequencyForX(float x, float minFrequencyHz, float maxFrequencyHz,
                        const juce::Rectangle<float> &plotBounds) const;

    /**
     * Maps a dB value onto the vertical plot range
     */
    float yForDb(float decibels, float minDb, float maxDb, const juce::Rectangle<float> &plotBounds) const;

    /**
     * Returns the hovered bar index for equal-width bar drawing
     */
    std::optional<size_t> bandIndexAt(juce::Point<float> position, size_t bandCount,
                                      const juce::Rectangle<float> &plotBounds) const;

    /**
     * Returns the draw bounds for one analyzer bar
     */
    juce::Rectangle<float> getBarBounds(size_t bandIndex, size_t bandCount, float displayedDb, float minDb, float maxDb,
                                        const juce::Rectangle<float> &plotBounds) const;

    /**
     * Returns the tooltip bounds clamped to the component area
     */
    juce::Rectangle<float> getTooltipBounds(juce::Point<float> hoverPosition, const juce::Rectangle<float> &plotBounds,
                                            const juce::Rectangle<float> &localBounds) const;
};
