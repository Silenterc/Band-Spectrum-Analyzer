#pragma once

#include <juce_core/juce_core.h>

namespace ParamIDs {
    inline constexpr const char* bandMode     = "bandMode";     // 30 / 40 / 60
    inline constexpr const char* freeze       = "freeze";

    inline constexpr const char* showRms  = "showRms";
    inline constexpr const char* showPeak = "showPeak";
    inline constexpr const char* showHold = "showHold";

    inline constexpr const char* holdMs     = "holdMs";
    inline constexpr const char* gridMinDb  = "gridMinDb";
    inline constexpr const char* gridMaxDb  = "gridMaxDb";
    inline constexpr const char* gridStepDb = "gridStepDb";

    inline juce::String signalSlotEnabled(const int slotIndex)   { return "signalSlot" + juce::String(slotIndex) + "Enabled"; }
    inline juce::String signalSlotVisible(const int slotIndex)   { return "signalSlot" + juce::String(slotIndex) + "Visible"; }
    inline juce::String signalSlotSource(const int slotIndex)    { return "signalSlot" + juce::String(slotIndex) + "Source"; }
    inline juce::String signalSlotMode(const int slotIndex)      { return "signalSlot" + juce::String(slotIndex) + "Mode"; }
    inline juce::String signalSlotColour(const int slotIndex)    { return "signalSlot" + juce::String(slotIndex) + "Colour"; }
    inline juce::String signalSlotOpacity(const int slotIndex)   { return "signalSlot" + juce::String(slotIndex) + "Opacity"; }
}
