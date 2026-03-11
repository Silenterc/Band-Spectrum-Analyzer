#include "SignalRackLayoutEngine.h"

SignalRackLayout SignalRackLayoutEngine::build(const juce::Rectangle<float> &rackBounds,
                                               const std::vector<SignalRackItemSpec> &items,
                                               const float gap) const {
    SignalRackLayout layout;
    layout.entries.reserve(items.size());

    auto x = rackBounds.getX();
    const auto y = rackBounds.getY();
    const auto height = rackBounds.getHeight();

    for (const auto &item: items) {
        SignalRackLayoutEntry entry;
        entry.slotIndex = item.slotIndex;
        entry.bounds = {x, y, item.width, height};
        layout.entries.push_back(entry);
        x += item.width + gap;
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
