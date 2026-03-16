#pragma once

#include <vector>

#include "../../dsp/core/AnalyzerData.h"
#include "AnalyzerUiConstants.h"

/**
 * UI-only state that controls how analyzer data is presented
 */
struct AnalyzerViewState {
    // Whether the frequency range is zoomed away from the full analyzer range
    bool useCustomFrequencyRange = false;
    // Visible minimum frequency in Hz when zoom is enabled
    float visibleMinFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMinFrequencyHz;
    // Visible maximum frequency in Hz when zoom is enabled
    float visibleMaxFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMaxFrequencyHz;
    // Which traces are enabled for drawing
    std::vector<Analyzer::TraceKind> enabledTraces;
};
