#pragma once

#include <vector>

#include "dsp/core/AnalyzerData.h"
#include "ui/state/SignalSlotUiState.h"

class SignalSlotOrderModel final {
public:
    std::vector<size_t> getVisibleOrderedSlots(
        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
        const Shared::SignalSlotOrder &slotOrder) const;

    size_t getTraceOrder(Analyzer::TraceKind kind, const Shared::SignalSlotOrder &slotOrder) const;
};
