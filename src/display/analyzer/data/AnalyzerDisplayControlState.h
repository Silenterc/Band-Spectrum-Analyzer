#pragma once

#include <array>
#include <cmath>

#include "display/analyzer/data/AnalyzerMeterData.h"
#include "shared/DefaultParameterValues.h"
#include "shared/SignalSlotConfiguration.h"

struct AnalyzerDisplayControlState {
    Analyzer::MeterSettings meterSettings;
    float floorDb = Defaults::gridMinDb;
    bool globalFrozen = false;
    std::array<Analyzer::SignalSlotConfiguration, Shared::maxSignalSlots> slotConfigurations{};
    std::array<bool, Shared::maxSignalSlots> slotFrozen{};
    // Contribution flags affect semantic state like hold ownership, not visual styling like order or opacity.
    std::array<bool, Shared::maxSignalSlots> slotContributing{};

    bool operator==(const AnalyzerDisplayControlState &other) const {
        return meterSettings.showRms == other.meterSettings.showRms
               && meterSettings.showPeak == other.meterSettings.showPeak
               && meterSettings.showHold == other.meterSettings.showHold
               && std::abs(meterSettings.holdMs - other.meterSettings.holdMs) <= 0.0001f
               && std::abs(meterSettings.rmsWindowMs - other.meterSettings.rmsWindowMs) <= 0.0001f
               && std::abs(meterSettings.peakDecayDbPerSecond - other.meterSettings.peakDecayDbPerSecond) <= 0.0001f
               && std::abs(meterSettings.holdDecayDbPerSecond - other.meterSettings.holdDecayDbPerSecond) <= 0.0001f
               && std::abs(floorDb - other.floorDb) <= 0.0001f
               && globalFrozen == other.globalFrozen
               && slotConfigurations == other.slotConfigurations
               && slotFrozen == other.slotFrozen
               && slotContributing == other.slotContributing;
    }

    bool operator!=(const AnalyzerDisplayControlState &other) const {
        return !(*this == other);
    }
};
