#include "SignalSlotOrderState.h"

SignalSlotOrderState::SignalSlotOrderState()
    : slotOrder(makeDefaultOrder()) {
}

const Shared::SignalSlotOrder &SignalSlotOrderState::getOrder() const {
    return slotOrder;
}

void SignalSlotOrderState::setOrder(const Shared::SignalSlotOrder &newSlotOrder) {
    const auto normalisedOrder = normalise(newSlotOrder);
    if (slotOrder == normalisedOrder)
        return;

    slotOrder = normalisedOrder;
    notifyListeners();
}

void SignalSlotOrderState::writeTo(juce::ValueTree &state) const {
    for (size_t orderIndex = 0; orderIndex < slotOrder.size(); ++orderIndex)
        state.setProperty(getPropertyId(orderIndex), static_cast<int>(slotOrder[orderIndex]), nullptr);
}

void SignalSlotOrderState::readFrom(const juce::ValueTree &state) {
    auto restoredOrder = makeDefaultOrder();

    for (size_t orderIndex = 0; orderIndex < restoredOrder.size(); ++orderIndex) {
        const auto propertyId = getPropertyId(orderIndex);
        if (state.hasProperty(propertyId))
            restoredOrder[orderIndex] = static_cast<size_t>(static_cast<int>(state[propertyId]));
    }

    const auto normalisedOrder = normalise(restoredOrder);
    if (slotOrder == normalisedOrder)
        return;

    slotOrder = normalisedOrder;
    notifyListeners();
}

void SignalSlotOrderState::addListener(Listener &listener) {
    listeners.add(&listener);
}

void SignalSlotOrderState::removeListener(Listener &listener) {
    listeners.remove(&listener);
}

Shared::SignalSlotOrder SignalSlotOrderState::makeDefaultOrder() {
    Shared::SignalSlotOrder defaultOrder{};

    for (size_t slotIndex = 0; slotIndex < defaultOrder.size(); ++slotIndex)
        defaultOrder[slotIndex] = slotIndex;

    return defaultOrder;
}

Shared::SignalSlotOrder SignalSlotOrderState::normalise(const Shared::SignalSlotOrder &sourceOrder) {
    Shared::SignalSlotOrder normalisedOrder{};
    std::array<bool, Shared::maxSignalSlots> seen{};
    size_t writeIndex = 0;

    for (const auto slotIndex: sourceOrder) {
        if (slotIndex >= Shared::maxSignalSlots || seen[slotIndex])
            continue;

        normalisedOrder[writeIndex++] = slotIndex;
        seen[slotIndex] = true;
    }

    for (size_t slotIndex = 0; slotIndex < seen.size(); ++slotIndex) {
        if (seen[slotIndex])
            continue;

        normalisedOrder[writeIndex++] = slotIndex;
    }

    return normalisedOrder;
}

juce::Identifier SignalSlotOrderState::getPropertyId(const size_t orderIndex) {
    return juce::Identifier("slotOrder" + juce::String(static_cast<int>(orderIndex)));
}

void SignalSlotOrderState::notifyListeners() {
    listeners.call([](Listener &listener) {
        listener.signalSlotOrderChanged();
    });
}
