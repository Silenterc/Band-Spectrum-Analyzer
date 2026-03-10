#pragma once

#include <juce_graphics/juce_graphics.h>

namespace Ui {
    enum class AccentPalette {
        blue,
        green,
        orange,
        purple
    };

    struct Theme {
        juce::Colour editorBackground;
        juce::Colour analyzerBackground;
        juce::Colour plotBackground;
        juce::Colour gridBorder;
        juce::Colour gridLine;
        juce::Colour axisText;
        juce::Colour tooltipBackground;
        juce::Colour tooltipBorder;
        juce::Colour tooltipText;
        juce::Colour rmsBarTop;
        juce::Colour rmsBarBottom;
        juce::Colour hoveredRmsBarTop;
        juce::Colour hoveredRmsBarBottom;
        juce::Colour barTop;
        juce::Colour barBottom;
        juce::Colour hoveredBarTop;
        juce::Colour hoveredBarBottom;
    };

    Theme makeTheme(AccentPalette accentPalette);
}
