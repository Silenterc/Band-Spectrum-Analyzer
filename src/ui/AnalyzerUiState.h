#pragma once

#include <array>
#include <cmath>

#include "SignalSlotUiState.h"
#include "analyzer/AnalyzerRenderData.h"

namespace Ui {
    struct AnalyzerUiState {
        std::array<SignalSlotState, Shared::maxSignalSlots> signalSlots{};
        Shared::SignalSlotOrder slotOrder{};
        Analyzer::MeterSettings meterSettings;
        bool frozen = false;
        bool sidechainAvailable = false;
    };

    inline bool operator==(const AnalyzerUiState &lhs, const AnalyzerUiState &rhs) {
        return lhs.signalSlots == rhs.signalSlots
               && lhs.slotOrder == rhs.slotOrder
               && lhs.meterSettings.showRms == rhs.meterSettings.showRms
               && lhs.meterSettings.showPeak == rhs.meterSettings.showPeak
               && lhs.meterSettings.showHold == rhs.meterSettings.showHold
               && std::abs(lhs.meterSettings.holdMs - rhs.meterSettings.holdMs) <= 0.0001f
               && lhs.frozen == rhs.frozen
               && lhs.sidechainAvailable == rhs.sidechainAvailable;
    }

    inline bool operator!=(const AnalyzerUiState &lhs, const AnalyzerUiState &rhs) {
        return !(lhs == rhs);
    }
}
