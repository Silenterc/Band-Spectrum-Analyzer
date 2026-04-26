#pragma once

#include <array>
#include <cmath>

#include "display/analyzer/data/AnalyzerMeterData.h"
#include "ui/analyzer/plot/state/AnalyzerUiConstants.h"
#include "ui/analyzer/rack/state/SignalSlotUiState.h"

namespace Ui {
    inline bool meterSettingsEqual(const Analyzer::MeterSettings &lhs, const Analyzer::MeterSettings &rhs) {
        return lhs.showRms == rhs.showRms
               && lhs.showPeak == rhs.showPeak
               && lhs.showHold == rhs.showHold
               && std::abs(lhs.holdMs - rhs.holdMs) <= 0.0001f
               && std::abs(lhs.rmsWindowMs - rhs.rmsWindowMs) <= 0.0001f
               && std::abs(lhs.peakDecayDbPerSecond - rhs.peakDecayDbPerSecond) <= 0.0001f
               && std::abs(lhs.holdDecayDbPerSecond - rhs.holdDecayDbPerSecond) <= 0.0001f;
    }

    /**
     * Snapshot of the UI state except the bars/graph
     */
    struct AnalyzerUiSnapshot {
        std::array<SignalSlotState, Shared::maxSignalSlots> signalSlots{};
        Shared::SignalSlotOrder slotOrder{};
        Analyzer::MeterSettings meterSettings;
        bool frozen = false;
        bool sidechainAvailable = false;
        float gridMinDb = 0.0f;
        float gridMaxDb = 0.0f;
        float gridStepDb = 0.0f;
        bool useCustomFrequencyRange = false;
        float visibleMinFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMinFrequencyHz;
        float visibleMaxFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMaxFrequencyHz;
    };

    inline bool operator==(const AnalyzerUiSnapshot &lhs, const AnalyzerUiSnapshot &rhs) {
        return lhs.signalSlots == rhs.signalSlots
               && lhs.slotOrder == rhs.slotOrder
               && meterSettingsEqual(lhs.meterSettings, rhs.meterSettings)
               && lhs.frozen == rhs.frozen
               && lhs.sidechainAvailable == rhs.sidechainAvailable
               && std::abs(lhs.gridMinDb - rhs.gridMinDb) <= 0.0001f
               && std::abs(lhs.gridMaxDb - rhs.gridMaxDb) <= 0.0001f
               && std::abs(lhs.gridStepDb - rhs.gridStepDb) <= 0.0001f
               && lhs.useCustomFrequencyRange == rhs.useCustomFrequencyRange
               && std::abs(lhs.visibleMinFrequencyHz - rhs.visibleMinFrequencyHz) <= 0.0001f
               && std::abs(lhs.visibleMaxFrequencyHz - rhs.visibleMaxFrequencyHz) <= 0.0001f;
    }

    inline bool operator!=(const AnalyzerUiSnapshot &lhs, const AnalyzerUiSnapshot &rhs) {
        return !(lhs == rhs);
    }
}
