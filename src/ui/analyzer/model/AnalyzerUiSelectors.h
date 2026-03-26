#pragma once

#include <array>
#include <vector>

#include "../../../dsp/core/AnalyzerData.h"
#include "AnalyzerUiSnapshot.h"

namespace Ui {
    inline bool isTraceVisible(const Analyzer::TraceKind kind,
                               const std::array<SignalSlotState, Shared::maxSignalSlots> &signalSlots) {
        if (const auto slotIndex = Analyzer::slotIndexForTraceKind(kind); slotIndex.has_value()) {
            if (*slotIndex >= signalSlots.size())
                return false;

            const auto &slot = signalSlots[*slotIndex];
            return slot.configuration.enabled && slot.visible;
        }

        return true;
    }

    inline std::vector<Analyzer::TraceKind> collectVisibleTraceKinds(const AnalyzerUiSnapshot &snapshot) {
        std::vector<Analyzer::TraceKind> traceKinds;
        traceKinds.reserve(snapshot.signalSlots.size());

        for (size_t slotIndex = 0; slotIndex < snapshot.signalSlots.size(); ++slotIndex) {
            const auto &slot = snapshot.signalSlots[slotIndex];
            if (slot.configuration.enabled && slot.visible)
                traceKinds.push_back(Analyzer::traceKindForSlot(slotIndex));
        }

        return traceKinds;
    }
}
