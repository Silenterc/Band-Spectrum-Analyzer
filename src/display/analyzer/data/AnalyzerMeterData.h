#pragma once

#include <memory>
#include <vector>

#include "dsp/core/AnalyzerData.h"
#include "shared/DefaultParameterValues.h"

namespace Analyzer {
    /**
     * Render-ready analyzer values in dB for one trace
     */
    struct MeterFrame {
        std::vector<float> rmsDb;
        std::vector<float> peakDb;
    };

    /**
     * One meter-processed analyzer trace for a logical slot
     */
    struct MeterTrace {
        TraceKind kind = TraceKind::slot1;
        MeterFrame frame;
    };

    /**
     * Meter-processed analyzer values produced on the display worker
     */
    struct MeterData {
        std::shared_ptr<const std::vector<BandInfo>> bandInfo;
        std::vector<MeterTrace> traces;
    };

    /**
     * UI-controlled analyzer meter settings
     */
    struct MeterSettings {
        bool showRms = true;
        bool showPeak = true;
        bool showHold = false;
        float holdMs = 0.0f;
    };
}
