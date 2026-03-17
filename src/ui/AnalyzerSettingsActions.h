#pragma once

#include <array>

#include "../shared/SignalSlotConfiguration.h"
#include "SignalSlotUiState.h"

class AnalyzerSettingsActions {
public:
    virtual ~AnalyzerSettingsActions() = default;

    virtual void setFreezeEnabled(bool isFrozen) = 0;
    virtual void setSignalSlotEnabled(size_t slotIndex, bool isEnabled) = 0;
    virtual void setSignalSlotVisible(size_t slotIndex, bool isVisible) = 0;
    virtual void setSignalSlotFrozen(size_t slotIndex, bool isFrozen) = 0;
    virtual void setSignalSlotSource(size_t slotIndex, Analyzer::SignalSource source) = 0;
    virtual void setSignalSlotMode(size_t slotIndex, Analyzer::SignalMode mode) = 0;
    virtual void setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) = 0;
    virtual void setSignalSlotColour(size_t slotIndex, int colourIndex) = 0;
    virtual void setSignalSlotOpacity(size_t slotIndex, float opacity) = 0;
    virtual void setShowPeakEnabled(bool isEnabled) = 0;
    virtual void setShowRmsEnabled(bool isEnabled) = 0;
    virtual void setShowHoldEnabled(bool isEnabled) = 0;

    virtual void setSignalSlotSignal(size_t slotIndex, Analyzer::SignalSource source, Analyzer::SignalMode mode) {
        setSignalSlotSource(slotIndex, source);
        setSignalSlotMode(slotIndex, mode);
    }

    virtual void applySignalSlotState(size_t slotIndex, const Ui::SignalSlotState &state) {
        setSignalSlotSignal(slotIndex, state.configuration.source, state.configuration.mode);
        setSignalSlotColour(slotIndex, state.colourIndex);
        setSignalSlotOpacity(slotIndex, state.opacity);
        setSignalSlotVisible(slotIndex, state.visible);
        setSignalSlotFrozen(slotIndex, state.frozen);
        setSignalSlotEnabled(slotIndex, state.configuration.enabled);
    }

    virtual void removeSignalSlot(size_t slotIndex) {
        setSignalSlotEnabled(slotIndex, false);
    }

    virtual void addSignalSlot(size_t slotIndex, const Ui::SignalSlotState &state, const Shared::SignalSlotOrder &slotOrder) {
        applySignalSlotState(slotIndex, state);
        setSignalSlotOrder(slotOrder);
    }
};
