#pragma once

namespace Analyzer::Constants {
    inline constexpr float minFrequencyHz = 20.0f;
    inline constexpr float maxAnalysisFractionOfNyquist = 0.9f;
    // Fixed analyzer frame size used for publishing stable, block-size-independent measurements.
    inline constexpr size_t analysisFrameSamples = 2048;
}
