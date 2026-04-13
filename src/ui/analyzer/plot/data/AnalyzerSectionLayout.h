#pragma once

#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/analyzer/plot/data/AnalyzerUiConstants.h"
#include "ui/analyzer/plot/logic/AnalyzerVisibleBandLayout.h"

struct AnalyzerGridLineLayout {
    float sectionY = 0.0f;
    float plotLocalY = 0.0f;
    juce::String label;
};

struct AnalyzerFrequencyMarkerLayout {
    float sectionX = 0.0f;
    float plotLocalX = 0.0f;
    juce::String label;
};

struct AnalyzerSectionLayout {
    juce::Rectangle<float> sectionBounds;
    juce::Rectangle<float> displayBounds;
    juce::Rectangle<float> plotBounds;
    juce::Rectangle<float> plotLocalBounds;
    juce::Rectangle<float> plotFrameBounds;
    std::vector<AnalyzerGridLineLayout> gridLines;
    std::vector<AnalyzerFrequencyMarkerLayout> frequencyMarkers;
    std::vector<AnalyzerVisibleBandLayout> sectionVisibleBands;
    std::vector<AnalyzerVisibleBandLayout> plotVisibleBands;
    float gridMinDb = 0.0f;
    float gridMaxDb = 0.0f;
    float gridStepDb = 0.0f;
    bool useCustomFrequencyRange = false;
    float visibleMinFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMinFrequencyHz;
    float visibleMaxFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMaxFrequencyHz;
};
