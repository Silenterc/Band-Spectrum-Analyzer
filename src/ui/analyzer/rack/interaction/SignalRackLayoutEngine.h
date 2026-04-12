#pragma once

#include <array>
#include <vector>

#include <juce_graphics/juce_graphics.h>

#include "shared/SignalSlotConfiguration.h"

struct SignalRackLayoutEntry {
    size_t slotIndex = 0;
    juce::Rectangle<float> bounds;
};

struct SignalRackItemSpec {
    size_t slotIndex = 0;
};

struct SignalRackLayout {
    std::vector<SignalRackLayoutEntry> entries;
    std::array<juce::Rectangle<float>, Shared::maxSignalSlots> laneBounds{};
    std::array<juce::Rectangle<float>, Shared::maxSignalSlots - 1> dividerBounds{};
    juce::Rectangle<float> activeSpan;
};

class SignalRackLayoutEngine final {
public:
    SignalRackLayout build(const juce::Rectangle<float> &rackBounds,
                           const std::vector<SignalRackItemSpec> &items,
                           float dividerThickness) const;
};
