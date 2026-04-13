#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "dsp/core/AnalyzerData.h"
#include "ui/theme/UiTheme.h"

/**
 * Converts analyzer data between domain space and screen space
 */
class AnalyzerGeometry final {
public:
    explicit AnalyzerGeometry(const Ui::Theme &themeToUse)
        : theme(themeToUse) {
    }

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
     * Maps a cursor y position back into a dB value on the same vertical scale
     */
    float dbForY(float y, float minDb, float maxDb, const juce::Rectangle<float> &plotBounds) const;

    /**
     * Returns the continuous hit bounds for one analyzer band mapped onto the visible frequency span.
     */
    juce::Rectangle<float> getBandHitBounds(float lowFrequencyHz,
                                            float highFrequencyHz,
                                            float visibleMinFrequencyHz,
                                            float visibleMaxFrequencyHz,
                                            const juce::Rectangle<float> &plotBounds) const;

    /**
     * Returns the tooltip bounds clamped to the component area
     */
    juce::Rectangle<float> getTooltipBounds(juce::Point<float> hoverPosition, const juce::Rectangle<float> &plotBounds,
                                            const juce::Rectangle<float> &localBounds) const;

private:
    const Ui::Theme &theme;
};
