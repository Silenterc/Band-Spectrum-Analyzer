#pragma once

#include <vector>

#include "../../dsp/core/AnalyzerData.h"

namespace Analyzer {
    /**
     * Render-ready analyzer values in dB for one trace
     */
    struct RenderFrame {
        // RMS level per band in dB
        std::vector<float> rmsDb;
        // Peak level per band in dB
        std::vector<float> peakDb;
    };

    /**
     * One render-ready analyzer trace after display-rate metering
     */
    struct RenderTrace {
        // Identity of this trace
        TraceKind kind = TraceKind::slot1;
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
        // Whether the global hold overlay should be shown
        bool showHold = false;
        // How long the global hold overlay stays pinned before it starts falling
        float holdMs = 0.0f;
    };
}
