#pragma once

namespace Analyzer::Constants {
    inline constexpr float minFrequencyHz = 20.0f;
    // Equal-tempered anchor used to align fractional-octave band centers to notes.
    inline constexpr double equalTemperedAnchorHz = 20.601722307054366;
    inline constexpr float maxAnalysisFractionOfNyquist = 0.9f;
    // Fixed analyzer frame size used for publishing stable, block-size-independent measurements.
    inline constexpr size_t analysisFrameSamples = 2048;
    // 50% overlap keeps transient alignment more stable without coupling DSP to the UI timer.
    inline constexpr size_t analysisHopSamples = analysisFrameSamples / 2;
}
