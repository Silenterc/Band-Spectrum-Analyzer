#include "SignalSlotOrderModel.h"

#include <algorithm>

std::vector<size_t> SignalSlotOrderModel::getVisibleOrderedSlots(
    const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
    const Shared::SignalSlotOrder &slotOrder) const {
    std::vector<size_t> visibleSlots;
    visibleSlots.reserve(signalSlots.size());

    for (const auto slotIndex: slotOrder) {
        if (slotIndex < signalSlots.size() && signalSlots[slotIndex].configuration.enabled)
            visibleSlots.push_back(slotIndex);
    }

    return visibleSlots;
}

size_t SignalSlotOrderModel::getTraceOrder(const Analyzer::TraceKind kind,
                                           const Shared::SignalSlotOrder &slotOrder) const {
    if (const auto slotIndex = Analyzer::slotIndexForTraceKind(kind); slotIndex.has_value()) {
        const auto orderIterator = std::find(slotOrder.begin(), slotOrder.end(), *slotIndex);
        if (orderIterator != slotOrder.end())
            return static_cast<size_t>(std::distance(slotOrder.begin(), orderIterator));
    }

    return slotOrder.size();
}
