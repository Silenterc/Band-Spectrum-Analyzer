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
     * Latest analyzer output for all bands in one engine
     */
    struct Frame {
        // RMS level per band in dB
        std::vector<float> rmsDb;
        // Peak level per band in dB
        std::vector<float> peakDb;
        // Peak hold level per band in dB
        std::vector<float> holdDb;
    };

    /**
     * Published analyzer state for one engine
     */
    struct EngineSnapshot {
        // Band layout that matches the current frame
        std::vector<BandInfo> bandInfo;
        // Latest analyzer values
        Frame frame;
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
     * One analyzer trace inside the composed UI snapshot
     */
    struct TraceSnapshot {
        // Identity of this trace
        TraceKind kind = TraceKind::input;
        // Analyzer values for this trace
        Frame frame;
    };

    /**
     * UI-facing analyzer state composed from one or more engines
     */
    struct CompositeSnapshot {
        // Shared band layout used by all visible traces
        std::vector<BandInfo> bandInfo;
        // All currently published traces for the UI
        std::vector<TraceSnapshot> traces;
    };
}
