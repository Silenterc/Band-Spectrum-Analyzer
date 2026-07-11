#pragma once

namespace Display::Constants {
    struct MeterTuning {
        float holdResetToleranceDb = 1.0f;
        float settleToleranceDb = 0.1f;
    };

    inline constexpr MeterTuning meterTuning{};

    inline constexpr double framesPerSecond = 50.0;
    static_assert(framesPerSecond > 0.0);
    inline constexpr double frameIntervalMs = 1000.0 / framesPerSecond;
    inline constexpr double idlePollIntervalMs = 120.0;
    inline constexpr float frameIntervalSeconds = 1.0f / static_cast<float>(framesPerSecond);
    inline constexpr int workerStopTimeoutMs = 2000;
}
