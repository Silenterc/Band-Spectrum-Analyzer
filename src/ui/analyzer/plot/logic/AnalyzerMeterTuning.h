#pragma once

namespace Ui {
    struct AnalyzerMeterTuning {
        float holdResetToleranceDb = 1.0f;
        float settleToleranceDb = 0.1f;
    };

    inline constexpr AnalyzerMeterTuning analyzerMeterTuning{};
}
