#pragma once

/** Display-rate tuning: meter/hold tolerances and worker wake cadence. */
namespace Display {
    struct AnalyzerMeterTuning {
        float holdResetToleranceDb = 1.0f;
        float settleToleranceDb = 0.1f;
    };

    inline constexpr AnalyzerMeterTuning analyzerMeterTuning{};

    inline constexpr int meterPollIntervalMs = 20; // 50 Hz, evenly paced for ProMotion displays
    inline constexpr int idleMeterPollIntervalMs = 120; // ~8 Hz while waiting for signal to return
    inline constexpr float meterPollIntervalSeconds = static_cast<float>(meterPollIntervalMs) * 0.001f;
}
