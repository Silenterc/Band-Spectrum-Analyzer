#pragma once

#include <vector>

namespace Analyzer {
    /**
     * Frequency bounds for a single analyzer band
     */
    struct BandInfo {
        // Lower edge of the band in Hz
        float lowHz = 0.0f;
        // Geometric center of the band in Hz
        float centerHz = 0.0f;
        // Upper edge of the band in Hz
        float highHz = 0.0f;
    };

    /**
     * Raw audio-rate measurement for one band over the latest engine window
     */
    struct BandMeasurements {
        // Maximum observed power y*y
        float peakPower = 0.0f;
        // Sum of power samples y*y used for mean-power calculation
        double sumPower = 0.0;
        // Number of samples that contributed to the sum
        int numSamples = 0;
    };

    /**
     * Identifies one logical analyzer trace in the UI
     */
    enum class TraceKind {
        input,
        sidechain,
        mid,
        side,
        custom
    };

    /**
     * One raw analyzer trace before display-rate metering
     */
    struct RawTrace {
        // Identity of this trace
        TraceKind kind = TraceKind::input;
        // Raw per-band measurements
        std::vector<BandMeasurements> measurements;
    };

}
