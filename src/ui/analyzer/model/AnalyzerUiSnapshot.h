#pragma once

#include <array>
#include <cmath>

#include "../AnalyzerRenderData.h"
#include "../../SignalSlotUiState.h"

namespace Ui {
    inline bool meterSettingsEqual(const Analyzer::MeterSettings &lhs, const Analyzer::MeterSettings &rhs) {
        return lhs.showRms == rhs.showRms
               && lhs.showPeak == rhs.showPeak
               && lhs.showHold == rhs.showHold
               && std::abs(lhs.holdMs - rhs.holdMs) <= 0.0001f;
    }

    struct AnalyzerUiSnapshot {
        std::array<SignalSlotState, Shared::maxSignalSlots> signalSlots{};
        Shared::SignalSlotOrder slotOrder{};
        Analyzer::MeterSettings meterSettings;
        bool frozen = false;
        bool sidechainAvailable = false;
        float gridMinDb = 0.0f;
        float gridMaxDb = 0.0f;
        float gridStepDb = 0.0f;
    };

    inline bool operator==(const AnalyzerUiSnapshot &lhs, const AnalyzerUiSnapshot &rhs) {
        return lhs.signalSlots == rhs.signalSlots
               && lhs.slotOrder == rhs.slotOrder
               && meterSettingsEqual(lhs.meterSettings, rhs.meterSettings)
               && lhs.frozen == rhs.frozen
               && lhs.sidechainAvailable == rhs.sidechainAvailable
               && std::abs(lhs.gridMinDb - rhs.gridMinDb) <= 0.0001f
               && std::abs(lhs.gridMaxDb - rhs.gridMaxDb) <= 0.0001f
               && std::abs(lhs.gridStepDb - rhs.gridStepDb) <= 0.0001f;
    }

    inline bool operator!=(const AnalyzerUiSnapshot &lhs, const AnalyzerUiSnapshot &rhs) {
        return !(lhs == rhs);
    }
}
