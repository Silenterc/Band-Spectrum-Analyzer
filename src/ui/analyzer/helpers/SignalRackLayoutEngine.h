#pragma once

#include <vector>

#include <juce_graphics/juce_graphics.h>

struct SignalRackLayoutEntry {
    size_t slotIndex = 0;
    juce::Rectangle<float> bounds;
};

struct SignalRackItemSpec {
    size_t slotIndex = 0;
    float width = 0.0f;
};

struct SignalRackLayout {
    std::vector<SignalRackLayoutEntry> entries;
    juce::Rectangle<float> activeSpan;
};

class SignalRackLayoutEngine final {
public:
    SignalRackLayout build(const juce::Rectangle<float> &rackBounds,
                           const std::vector<SignalRackItemSpec> &items,
                           float gap) const;
};
