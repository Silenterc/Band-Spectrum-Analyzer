#pragma once

#include <juce_core/juce_core.h>

/**
 * Converts analyzer frequencies into note names
 */
class MusicTheory final {
public:
    /**
     * Returns the nearest equal-tempered note for the given frequency
     */
    juce::String getNearestNoteName(float frequencyHz) const;
};
