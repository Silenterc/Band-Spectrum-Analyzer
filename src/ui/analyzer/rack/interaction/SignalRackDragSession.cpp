#include "SignalRackDragSession.h"

#include <algorithm>

#include <juce_core/juce_core.h>

void SignalRackDragSession::begin(const size_t slotIndex,
                                  const Shared::SignalSlotOrder &fullOrder,
                                  const std::vector<size_t> &visibleOrder,
                                  const SignalRackLayout &layout,
                                  const float mouseX) {
    const auto draggedEntry = findEntry(layout, slotIndex);
    if (!draggedEntry.has_value())
        return;

    draggedSlotIndex = slotIndex;
    baseFullOrder = fullOrder;
    previewFullOrder = fullOrder;
    previewVisibleOrder = visibleOrder;
    draggedBounds = draggedEntry->bounds;
    previousDraggedX = draggedBounds.getX();
    currentDragDirection = 0;
    dragOffsetX = mouseX - draggedBounds.getX();
}

SignalRackDragSession::UpdateResult SignalRackDragSession::update(const float mouseX, const SignalRackLayout &layout) {
    UpdateResult result;
    if (!draggedSlotIndex.has_value())
        return result;

    const auto previousDraggedBounds = draggedBounds;
    const auto previousPreviewOrder = previewFullOrder;

    if (layout.activeSpan.getWidth() <= 0.0f) {
        recomputePreview(layout);
        result.previewOrderChanged = previewFullOrder != previousPreviewOrder;
        return result;
    }

    const auto minX = layout.activeSpan.getX();
    const auto maxX = layout.activeSpan.getRight() - draggedBounds.getWidth();
    const auto newDraggedX = juce::jlimit(minX, maxX, mouseX - dragOffsetX);

    if (newDraggedX > previousDraggedX + 0.5f)
        currentDragDirection = 1;
    else if (newDraggedX < previousDraggedX - 0.5f)
        currentDragDirection = -1;

    previousDraggedX = newDraggedX;
    draggedBounds.setX(newDraggedX);
    draggedBounds.setY(layout.activeSpan.getY());
    draggedBounds.setHeight(layout.activeSpan.getHeight());
    recomputePreview(layout);

    result.draggedBoundsChanged = draggedBounds != previousDraggedBounds;
    result.previewOrderChanged = previewFullOrder != previousPreviewOrder;
    return result;
}

std::optional<Shared::SignalSlotOrder> SignalRackDragSession::finish() {
    if (!draggedSlotIndex.has_value())
        return std::nullopt;

    const auto result = previewFullOrder;
    cancel();
    return result;
}

void SignalRackDragSession::cancel() {
    draggedSlotIndex.reset();
    previewVisibleOrder.clear();
    previousDraggedX = 0.0f;
    currentDragDirection = 0;
}

bool SignalRackDragSession::isDragging() const {
    return draggedSlotIndex.has_value();
}

std::optional<size_t> SignalRackDragSession::getDraggedSlotIndex() const {
    return draggedSlotIndex;
}

juce::Rectangle<float> SignalRackDragSession::getDraggedBounds() const {
    return draggedBounds;
}

Shared::SignalSlotOrder SignalRackDragSession::getDisplayOrder(const Shared::SignalSlotOrder &persistedOrder) const {
    return isDragging() ? previewFullOrder : persistedOrder;
}

void SignalRackDragSession::recomputePreview(const SignalRackLayout &layout) {
    if (!draggedSlotIndex.has_value())
        return;

    std::vector<size_t> reorderedVisibleSlots;
    reorderedVisibleSlots.reserve(previewVisibleOrder.size());

    for (const auto slotIndex: previewVisibleOrder) {
        if (slotIndex != *draggedSlotIndex)
            reorderedVisibleSlots.push_back(slotIndex);
    }

    const auto thresholdX = currentDragDirection > 0 ? draggedBounds.getRight()
                            : currentDragDirection < 0 ? draggedBounds.getX()
                                                             : draggedBounds.getCentreX();
    size_t insertionIndex = 0;

    for (const auto &entry: layout.entries) {
        if (entry.slotIndex == *draggedSlotIndex)
            continue;

        if (thresholdX >= entry.bounds.getCentreX())
            ++insertionIndex;
    }

    insertionIndex = std::min(insertionIndex, reorderedVisibleSlots.size());
    reorderedVisibleSlots.insert(reorderedVisibleSlots.begin() + static_cast<std::ptrdiff_t>(insertionIndex),
                                 *draggedSlotIndex);

    previewVisibleOrder = reorderedVisibleSlots;
    previewFullOrder = buildFullPreviewOrder();
}

std::optional<SignalRackLayoutEntry> SignalRackDragSession::findEntry(const SignalRackLayout &layout,
                                                                      const size_t slotIndex) {
    const auto iterator = std::find_if(layout.entries.begin(), layout.entries.end(),
                                       [slotIndex](const SignalRackLayoutEntry &entry) {
                                           return entry.slotIndex == slotIndex;
                                       });
    if (iterator == layout.entries.end())
        return std::nullopt;

    return *iterator;
}

Shared::SignalSlotOrder SignalRackDragSession::buildFullPreviewOrder() const {
    auto fullPreviewOrder = baseFullOrder;
    size_t visibleIndex = 0;

    for (size_t orderIndex = 0; orderIndex < fullPreviewOrder.size(); ++orderIndex) {
        const auto slotIndex = baseFullOrder[orderIndex];
        if (std::find(previewVisibleOrder.begin(), previewVisibleOrder.end(), slotIndex) == previewVisibleOrder.end())
            continue;

        fullPreviewOrder[orderIndex] = previewVisibleOrder[visibleIndex++];
    }

    return fullPreviewOrder;
}
