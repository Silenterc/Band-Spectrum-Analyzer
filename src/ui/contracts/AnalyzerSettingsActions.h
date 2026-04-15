#pragma once

#include <array>

#include "../../shared/SignalSlotConfiguration.h"
#include "../state/SignalSlotUiState.h"

class AnalyzerSettingsActions {
public:
    virtual ~AnalyzerSettingsActions() = default;

    virtual void setFreezeEnabled(bool isFrozen) = 0;
    virtual void setSignalSlotEnabled(size_t slotIndex, bool isEnabled) = 0;
    virtual void setSignalSlotVisible(size_t slotIndex, bool isVisible) = 0;
    virtual void setSignalSlotFrozen(size_t slotIndex, bool isFrozen) = 0;
    virtual void setSignalSlotSolo(size_t slotIndex, bool isSolo) = 0;
    virtual void setSignalSlotSource(size_t slotIndex, Analyzer::SignalSource source) = 0;
    virtual void setSignalSlotMode(size_t slotIndex, Analyzer::SignalMode mode) = 0;
    virtual void setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) = 0;
    virtual void setSignalSlotColour(size_t slotIndex, int colourIndex) = 0;
    virtual void setSignalSlotOpacity(size_t slotIndex, float opacity) = 0;
    virtual void setShowPeakEnabled(bool isEnabled) = 0;
    virtual void setShowRmsEnabled(bool isEnabled) = 0;
    virtual void setShowHoldEnabled(bool isEnabled) = 0;
    virtual void setCustomFrequencyRangeEnabled(bool isEnabled) = 0;
    virtual void setVisibleMinFrequencyHz(float frequencyHz) = 0;
    virtual void setVisibleMaxFrequencyHz(float frequencyHz) = 0;
    virtual void removeSignalSlot(size_t slotIndex) = 0;
    virtual void addSignalSlot(size_t slotIndex,
                               const Ui::SignalSlotState &state,
                               const Shared::SignalSlotOrder &slotOrder) = 0;
};
