#pragma once

#include <vector>

#include "../../dsp/AnalyzerData.h"

/**
 * UI-only state that controls how analyzer data is presented
 */
struct AnalyzerViewState {
    // Whether the frequency range is zoomed away from the full analyzer range
    bool useCustomFrequencyRange = false;
    // Visible minimum frequency in Hz when zoom is enabled
    float visibleMinFrequencyHz = 20.0f;
    // Visible maximum frequency in Hz when zoom is enabled
    float visibleMaxFrequencyHz = 20000.0f;
    // Which traces are enabled for drawing
    std::vector<Analyzer::TraceKind> enabledTraces;
};
