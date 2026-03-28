#pragma once

#include "../../../shared/DefaultParameterValues.h"

namespace Ui {
    struct AnalyzerMeterTuning {
        float rmsWindowMs = Defaults::rmsWindowMs;
        float rmsDecayDbPerSecond = Defaults::rmsDecayDbPerSecond;
        float peakDecayDbPerSecond = Defaults::peakDecayDbPerSecond;
        float holdDecayDbPerSecond = Defaults::holdDecayDbPerSecond;
        float holdResetToleranceDb = 1.5f;
        float settleToleranceDb = 0.1f;
    };

    inline constexpr AnalyzerMeterTuning analyzerMeterTuning{};
}
