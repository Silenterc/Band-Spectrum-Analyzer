#pragma once

#include "shared/AnalyzerProductConstants.h"

namespace Analyzer::Constants {
    inline constexpr float minFrequencyHz = Shared::AnalyzerProductConstants::minimumFrequencyHz;
    // Equal-tempered anchor used to align fractional-octave band centers to notes.
    inline constexpr double equalTemperedAnchorHz = 20.601722307054366;
    inline constexpr float maxAnalysisFractionOfNyquist = 0.9f;
    // Fixed analyzer frame size used for publishing stable, block-size-independent measurements.
    inline constexpr size_t analysisFrameSamples = 2048;
    // 50% overlap keeps transient alignment more stable without coupling DSP to the UI timer.
    inline constexpr size_t analysisHopSamples = analysisFrameSamples / 2;
    inline constexpr float activityActivateThresholdGain = 0.0000630957f;   // -84 dBFS
    inline constexpr float activityDeactivateThresholdGain = 0.0000316228f; // -90 dBFS
    inline constexpr float activitySilenceHoldMs = 220.0f;
    inline constexpr double defaultSampleRateHz = 44100.0;
}
