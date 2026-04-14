#pragma once

#include "../dsp/core/BandMode.h"
#include "SignalSlotConfiguration.h"

namespace Defaults {
    // Global analyzer parameter defaults
    inline constexpr Analyzer::BandMode bandMode = Analyzer::BandMode::octaveSixth;

    inline constexpr bool showRms = false;
    inline constexpr bool showPeak = true;
    inline constexpr bool showHold = true;
    inline constexpr bool freeze = false;

    inline constexpr float holdMs = 2000.0f;
    inline constexpr float gridMinDb = -50.0f;
    inline constexpr float gridMaxDb = 3.0f;
    inline constexpr float gridStepDb = 5.0f;
    inline constexpr bool useCustomFrequencyRange = false;
    inline constexpr float visibleMinFrequencyHz = 20.0f;
    inline constexpr float visibleMaxFrequencyHz = 20000.0f;

    // Global parameter ranges
    inline constexpr float holdMsMin = 0.0f;
    inline constexpr float holdMsMax = 5000.0f;
    inline constexpr float holdMsStep = 10.0f;

    inline constexpr float rmsWindowMsMin = 50.0f;
    inline constexpr float rmsWindowMsMax = 2000.0f;
    inline constexpr float rmsWindowMsStep = 10.0f;

    inline constexpr float gridMinDbMin = -120.0f;
    inline constexpr float gridMinDbMax = -12.0f;
    inline constexpr float gridMinDbStep = 1.0f;

    inline constexpr float gridMaxDbMin = -24.0f;
    inline constexpr float gridMaxDbMax = 24.0f;
    inline constexpr float gridMaxDbStep = 1.0f;

    inline constexpr float gridStepDbMin = 1.0f;
    inline constexpr float gridStepDbMax = 24.0f;
    inline constexpr float gridStepDbStep = 1.0f;

    inline constexpr float visibleMinFrequencyHzMin = 20.0f;
    inline constexpr float visibleMinFrequencyHzMax = 20000.0f;
    inline constexpr float visibleMinFrequencyHzStep = 1.0f;

    inline constexpr float visibleMaxFrequencyHzMin = 20.0f;
    inline constexpr float visibleMaxFrequencyHzMax = 20000.0f;
    inline constexpr float visibleMaxFrequencyHzStep = 1.0f;

    inline constexpr float signalOpacity = 0.78f;
    inline constexpr float signalOpacityMin = 0.15f;
    inline constexpr float signalOpacityMax = 1.0f;
    inline constexpr float signalOpacityStep = 0.01f;

    // Display-rate meter tuning
    inline constexpr float rmsWindowMs = 200.0f;
    inline constexpr float peakDecayDbPerSecond = 15.0f;
    inline constexpr float holdDecayDbPerSecond = 6.0f;

    inline constexpr bool isSignalSlotEnabled(const size_t slotIndex) {
        return slotIndex == 0;
    }

    inline constexpr bool isSignalSlotVisible(const size_t slotIndex) {
        (void) slotIndex;
        return true;
    }

    inline constexpr bool isSignalSlotFrozen(const size_t slotIndex) {
        (void) slotIndex;
        return false;
    }

    inline constexpr Analyzer::SignalSource signalSlotSource(const size_t slotIndex) {
        (void) slotIndex;
        return Analyzer::SignalSource::main;
    }

    inline constexpr Analyzer::SignalMode signalSlotMode(const size_t slotIndex) {
        return slotIndex == 0 ? Analyzer::SignalMode::stereo : Analyzer::SignalMode::mid;
    }

    inline constexpr int signalSlotColour(const size_t slotIndex) {
        return static_cast<int>(slotIndex);
    }
}
