#pragma once

#include <cmath>

#include "../plugin/ParamSpec.h"

namespace Analyzer {
    struct ParameterState {
        // Which channel view the analyzer should use: summed, mid/side or stereo
        ParamSpec::AnalysisMode analysisMode = ParamSpec::AnalysisMode::summed;
        // How many bars/bands the analyzer should build
        ParamSpec::BandMode bandMode = ParamSpec::BandMode::bands40;
        // Whether to expose RMS values in the output frame
        bool showRms = ParamSpec::defaultShowRms;
        // Whether to expose peak values in the output frame
        bool showPeak = ParamSpec::defaultShowPeak;
        // Whether to expose the held peak marker in the output frame
        bool showHold = ParamSpec::defaultShowHold;
        // How long the held peak stays pinned before it starts falling
        float holdMs = ParamSpec::defaultHoldMs;
        // Lower display floor in dB
        float gridMinDb = ParamSpec::defaultGridMinDb;
        // Upper display ceiling in dB
        float gridMaxDb = ParamSpec::defaultGridMaxDb;
        // Grid spacing in dB
        float gridStepDb = ParamSpec::defaultGridStepDb;
    };

    inline bool operator==(const ParameterState &lhs, const ParameterState &rhs) {
        return lhs.analysisMode == rhs.analysisMode
               && lhs.bandMode == rhs.bandMode
               && lhs.showRms == rhs.showRms
               && lhs.showPeak == rhs.showPeak
               && lhs.showHold == rhs.showHold
               && std::abs(lhs.holdMs - rhs.holdMs) <= 0.0001f
               && std::abs(lhs.gridMinDb - rhs.gridMinDb) <= 0.0001f
               && std::abs(lhs.gridMaxDb - rhs.gridMaxDb) <= 0.0001f
               && std::abs(lhs.gridStepDb - rhs.gridStepDb) <= 0.0001f;
    }

    inline bool operator!=(const ParameterState &lhs, const ParameterState &rhs) {
        return !(lhs == rhs);
    }
}
