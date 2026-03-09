#pragma once

#include <vector>

#include "../../dsp/AnalyzerData.h"

namespace Analyzer {
    /**
     * Render-ready analyzer values in dB for one trace
     */
    struct RenderFrame {
        // RMS level per band in dB
        std::vector<float> rmsDb;
        // Peak level per band in dB
        std::vector<float> peakDb;
        // Peak hold level per band in dB
        std::vector<float> holdDb;
    };

    /**
     * One render-ready analyzer trace after display-rate metering
     */
    struct RenderTrace {
        // Identity of this trace
        TraceKind kind = TraceKind::input;
        // Render-ready dB values
        RenderFrame frame;
    };

    /**
     * Render-ready analyzer data for the UI
     */
    struct RenderData {
        // Shared band layout used by all traces
        std::vector<BandInfo> bandInfo;
        // Render-ready traces
        std::vector<RenderTrace> traces;
    };

    /**
     * UI-controlled analyzer meter settings
     */
    struct MeterSettings {
        // Whether RMS bars should be shown
        bool showRms = true;
        // Whether peak caps should be shown
        bool showPeak = true;
        // Whether hold markers should be shown
        bool showHold = false;
        // How long hold markers stay pinned before they start falling
        float holdMs = 0.0f;
    };
}
