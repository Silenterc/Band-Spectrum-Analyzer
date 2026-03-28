#include "SignalRackLayoutEngine.h"

SignalRackLayout SignalRackLayoutEngine::build(const juce::Rectangle<float> &rackBounds,
                                               const std::vector<SignalRackItemSpec> &items,
                                               const float dividerThickness) const {
    SignalRackLayout layout;
    layout.entries.reserve(items.size());

    const auto y = rackBounds.getY();
    const auto height = rackBounds.getHeight();
    const auto dividerCount = static_cast<float>(Shared::maxSignalSlots - 1);
    const auto totalDividerWidth = dividerThickness * dividerCount;
    const auto laneWidth = juce::jmax(0.0f, (rackBounds.getWidth() - totalDividerWidth) / static_cast<float>(Shared::maxSignalSlots));

    auto x = rackBounds.getX();
    for (size_t laneIndex = 0; laneIndex < layout.laneBounds.size(); ++laneIndex) {
        layout.laneBounds[laneIndex] = {x, y, laneWidth, height};
        x += laneWidth;

        if (laneIndex < layout.dividerBounds.size()) {
            layout.dividerBounds[laneIndex] = {x, y, dividerThickness, height};
            x += dividerThickness;
        }
    }

    for (size_t itemIndex = 0; itemIndex < items.size() && itemIndex < layout.laneBounds.size(); ++itemIndex) {
        const auto &item = items[itemIndex];
        SignalRackLayoutEntry entry;
        entry.slotIndex = item.slotIndex;
        entry.bounds = layout.laneBounds[itemIndex];
        layout.entries.push_back(entry);
    }

    if (!layout.entries.empty()) {
        layout.activeSpan = {
            layout.entries.front().bounds.getX(),
            y,
            layout.entries.back().bounds.getRight() - layout.entries.front().bounds.getX(),
            height
        };
    } else {
        layout.activeSpan = {rackBounds.getX(), y, 0.0f, height};
    }

    return layout;
}
