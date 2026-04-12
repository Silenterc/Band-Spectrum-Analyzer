#pragma once

#include <juce_data_structures/juce_data_structures.h>

#include "../../shared/SignalSlotConfiguration.h"

class SignalSlotOrderState final {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void signalSlotOrderChanged() = 0;
    };

    SignalSlotOrderState();

    const Shared::SignalSlotOrder &getOrder() const;
    void setOrder(const Shared::SignalSlotOrder &slotOrder);

    void writeTo(juce::ValueTree &state) const;
    void readFrom(const juce::ValueTree &state);

    void addListener(Listener &listener);
    void removeListener(Listener &listener);

private:
    static Shared::SignalSlotOrder makeDefaultOrder();
    static Shared::SignalSlotOrder normalise(const Shared::SignalSlotOrder &slotOrder);
    static juce::Identifier getPropertyId(size_t orderIndex);
    void notifyListeners();

    Shared::SignalSlotOrder slotOrder;
    juce::ListenerList<Listener> listeners;
};
