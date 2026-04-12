#pragma once

#include <juce_core/juce_core.h>

/**
 * Formats analyzer values into user-facing frequency and level strings
 */
class FrequencyFormatter final {
public:
    /**
     * Formats axis labels into compact Hz / kHz text
     */
    juce::String formatScaleFrequency(float frequencyHz) const;

    /**
     * Formats hover frequency text rounded to the nearest 1 Hz
     */
    juce::String formatHoverFrequency(float frequencyHz) const;

    /**
     * Formats hover level text with one decimal place
     */
    juce::String formatDecibels(float decibels) const;
};
