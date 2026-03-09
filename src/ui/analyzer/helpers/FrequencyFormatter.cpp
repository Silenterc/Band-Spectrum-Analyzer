#include "FrequencyFormatter.h"

#include <cmath>

juce::String FrequencyFormatter::formatScaleFrequency(float frequencyHz) const {
    if (frequencyHz >= 1000.0f)
        return juce::String(frequencyHz / 1000.0f, 1) + "k";

    return juce::String(std::round(frequencyHz));
}

juce::String FrequencyFormatter::formatHoverFrequency(float frequencyHz) const {
    return juce::String(static_cast<int>(std::round(frequencyHz))) + " Hz";
}

juce::String FrequencyFormatter::formatDecibels(float decibels) const {
    return juce::String(decibels, 1) + " dB";
}
