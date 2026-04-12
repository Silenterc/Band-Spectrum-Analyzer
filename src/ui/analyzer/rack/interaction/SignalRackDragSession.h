#pragma once

#include <optional>
#include <vector>

#include "shared/SignalSlotConfiguration.h"
#include "SignalRackLayoutEngine.h"

class SignalRackDragSession final {
public:
    void begin(size_t slotIndex,
               const Shared::SignalSlotOrder &fullOrder,
               const std::vector<size_t> &visibleOrder,
               const SignalRackLayout &layout,
               float mouseX);

    void update(float mouseX, const SignalRackLayout &layout);

    std::optional<Shared::SignalSlotOrder> finish();
    void cancel();

    bool isDragging() const;
    std::optional<size_t> getDraggedSlotIndex() const;
    juce::Rectangle<float> getDraggedBounds() const;
    Shared::SignalSlotOrder getDisplayOrder(const Shared::SignalSlotOrder &persistedOrder) const;

private:
    void recomputePreview(const SignalRackLayout &layout);
    static std::optional<SignalRackLayoutEntry> findEntry(const SignalRackLayout &layout, size_t slotIndex);
    Shared::SignalSlotOrder buildFullPreviewOrder() const;

    std::optional<size_t> draggedSlotIndex;
    Shared::SignalSlotOrder baseFullOrder{};
    Shared::SignalSlotOrder previewFullOrder{};
    std::vector<size_t> previewVisibleOrder;
    juce::Rectangle<float> draggedBounds;
    float previousDraggedX = 0.0f;
    int currentDragDirection = 0;
    float dragOffsetX = 0.0f;
};
