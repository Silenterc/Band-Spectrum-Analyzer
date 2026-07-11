#pragma once

#include <cstddef>
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
        // Maximum observed instantaneous linear power over the block: max(y * y)
        // This is still in squared sample units, not dB.
        float peakPower = 0.0f;
        // Sum of instantaneous linear power over the newly advanced RMS hop: sum(y * y)
        // Divide by rmsHopNumSamples to get mean power before RMS / dB conversion.
        double rmsHopSumPower = 0.0;
        // Number of filtered samples that contributed to rmsHopSumPower
        int rmsHopNumSamples = 0;
    };

    /**
     * Identifies one logical analyzer trace; values map 1:1 to signal slot indices
     */
    enum class TraceKind {
        slot1,
        slot2,
        slot3,
        slot4
    };

    inline constexpr TraceKind traceKindForSlot(const size_t slotIndex) {
        return static_cast<TraceKind>(slotIndex);
    }

    inline constexpr size_t slotIndexForTraceKind(const TraceKind kind) {
        return static_cast<size_t>(kind);
    }

    /**
     * One raw analyzer trace before display-rate metering
     */
    struct RawTrace {
        // Identity of this trace
        TraceKind kind = TraceKind::slot1;
        // Raw per-band measurements
        std::vector<BandMeasurements> measurements;
    };
}
