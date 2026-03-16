#pragma once

#include "../dsp/core/BandMode.h"
#include "SignalSlotConfiguration.h"

namespace Defaults {
    inline constexpr Analyzer::BandMode bandMode = Analyzer::BandMode::bands30;

    inline constexpr bool showRms = false;
    inline constexpr bool showPeak = true;
    inline constexpr bool showHold = true;
    inline constexpr bool freeze = false;

    inline constexpr float holdMs = 500.0f;
    inline constexpr float gridMinDb = -50.0f;
    inline constexpr float gridMaxDb = 0.0f;
    inline constexpr float gridStepDb = 5.0f;

    inline constexpr float signalOpacity = 0.78f;

    inline constexpr bool isSignalSlotEnabled(const size_t slotIndex) {
        return slotIndex == 0;
    }

    inline constexpr bool isSignalSlotVisible(const size_t slotIndex) {
        (void) slotIndex;
        return true;
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
