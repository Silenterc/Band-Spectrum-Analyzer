#pragma once

#include "shared/DefaultParameterValues.h"

namespace Ui {
    struct AnalyzerMeterTuning {
        float peakDecayDbPerSecond = Defaults::peakDecayDbPerSecond;
        float holdDecayDbPerSecond = Defaults::holdDecayDbPerSecond;
        float holdResetToleranceDb = 1.0f;
        float settleToleranceDb = 0.1f;
    };

    inline constexpr AnalyzerMeterTuning analyzerMeterTuning{};
}
