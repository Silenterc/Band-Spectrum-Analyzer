#include "SignalRackComponent.h"

#include <algorithm>

#include "../model/SignalRackModel.h"

namespace {
    juce::Rectangle<int> getRackContentBounds(const juce::Rectangle<int> bounds, const Ui::Theme &theme) {
        auto rackBounds = bounds;
        rackBounds.removeFromTop(static_cast<int>(theme.metrics.rack.topInset));
        return rackBounds;
    }

    juce::Rectangle<int> getLaneModuleBounds(const juce::Rectangle<int> laneBounds, const Ui::Theme &theme) {
        auto moduleBounds = laneBounds;
        moduleBounds.removeFromBottom(static_cast<int>(theme.metrics.rack.bottomInset));
        return moduleBounds;
    }
}

SignalRackComponent::SignalRackComponent(AnalyzerUiSnapshotSource &uiSnapshotSourceToUse,
                                         AnalyzerSettingsActions &settingsActionsToUse,
                                         const Ui::Theme &themeToUse)
    : uiSnapshotSource(uiSnapshotSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse),
      addButton(themeToUse) {
    for (auto &slotDivider: slotDividers) {
        slotDivider = std::make_unique<SectionDividerComponent>(theme, SectionDividerComponent::Orientation::vertical);
        addAndMakeVisible(*slotDivider);
    }

    for (auto &slotComponent: slotComponents) {
        slotComponent = std::make_unique<SignalSlotComponent>(theme);
        addAndMakeVisible(*slotComponent);
        slotComponent->onSignalSelected = [this](const size_t slotIndex, const Analyzer::SignalSource source,
                                                 const Analyzer::SignalMode mode) {
            settingsActions.setSignalSlotSignal(slotIndex, source, mode);
        };
        slotComponent->onColourSelected = [this](const size_t slotIndex, const int colourIndex) {
            settingsActions.setSignalSlotColour(slotIndex, colourIndex);
        };
        slotComponent->onVisibilityChanged = [this](const size_t slotIndex, const bool isVisible) {
            settingsActions.setSignalSlotVisible(slotIndex, isVisible);
        };
        slotComponent->onFrozenChanged = [this](const size_t slotIndex, const bool isFrozen) {
            settingsActions.setSignalSlotFrozen(slotIndex, isFrozen);
        };
        slotComponent->onRemoveClicked = [this](const size_t slotIndex) {
            settingsActions.removeSignalSlot(slotIndex);
        };
        slotComponent->onOpacityChanged = [this](const size_t slotIndex, const float opacity) {
            settingsActions.setSignalSlotOpacity(slotIndex, opacity);
        };
        slotComponent->onReorderDragStarted = [this](const size_t slotIndex, const float startMouseX) {
            const auto persistedOrder = currentSnapshot.slotOrder;
            const auto visibleOrderedSlots = getVisibleOrderedSlots(persistedOrder);
            const auto layout = buildLayout(visibleOrderedSlots);
            dragSession.begin(slotIndex, persistedOrder, visibleOrderedSlots, layout, startMouseX);
            if (auto *draggedComponent = findComponentForSlot(slotIndex))
                draggedSnapshot = draggedComponent->createComponentSnapshot(draggedComponent->getLocalBounds());
            refreshFromState(true);
        };
        slotComponent->onReorderDragged = [this](const size_t slotIndex, const float xPosition) {
            juce::ignoreUnused(slotIndex);
            const auto previewOrder = dragSession.getDisplayOrder(currentSnapshot.slotOrder);
            const auto visibleOrderedSlots = getVisibleOrderedSlots(previewOrder);
            const auto layout = buildLayout(visibleOrderedSlots);
            dragSession.update(xPosition, layout);
            refreshFromState(true);
        };
        slotComponent->onReorderDragEnded = [this](const size_t slotIndex, const float xPosition) {
            juce::ignoreUnused(slotIndex);
            const auto previewOrder = dragSession.getDisplayOrder(currentSnapshot.slotOrder);
            const auto visibleOrderedSlots = getVisibleOrderedSlots(previewOrder);
            const auto layout = buildLayout(visibleOrderedSlots);
            dragSession.update(xPosition, layout);
            const auto reorderedSlots = dragSession.finish();
            if (reorderedSlots.has_value())
                settingsActions.setSignalSlotOrder(*reorderedSlots);
            draggedSnapshot = {};
            refreshFromState(true);
        };
    }

    addAndMakeVisible(addButton);
    addButton.onClick = [this] { addSignal(); };
    addButton.setTooltip("Add signal");
    SignalSlotActionButton::Style addStyle;
    addStyle.content = SignalSlotActionButton::Content::text;
    addStyle.text = "+";
    addStyle.fontHeight = theme.metrics.slot.addButtonFontHeight;
    addStyle.fill = juce::Colours::transparentBlack;
    addStyle.hoverFill = juce::Colours::transparentBlack;
    addStyle.foreground = theme.hardwareMarkingLight;
    addStyle.drawsBackground = false;
    addButton.setStyle(addStyle);
    uiSnapshotSource.addAnalyzerUiSnapshotListener(*this);
    analyzerUiSnapshotChanged(uiSnapshotSource.getAnalyzerUiSnapshot());
}

SignalRackComponent::~SignalRackComponent() {
    uiSnapshotSource.removeAnalyzerUiSnapshotListener(*this);
}

void SignalRackComponent::resized() {
    const auto displayOrder = dragSession.getDisplayOrder(currentSnapshot.slotOrder);
    const auto visibleOrderedSlots = getVisibleOrderedSlots(displayOrder);
    const auto layout = buildLayout(visibleOrderedSlots);
    const auto draggedSlotIndex = dragSession.getDraggedSlotIndex();

    for (size_t dividerIndex = 0; dividerIndex < slotDividers.size(); ++dividerIndex)
        slotDividers[dividerIndex]->setBounds(layout.dividerBounds[dividerIndex].toNearestInt());

    for (size_t slotIndex = 0; slotIndex < slotComponents.size(); ++slotIndex) {
        auto &slotComponent = slotComponents[slotIndex];
        if (!slotComponent->isVisible()) {
            slotComponent->setDragged(false);
            continue;
        }

        const auto entryIterator = std::find_if(layout.entries.begin(), layout.entries.end(),
                                                [slotIndex](const SignalRackLayoutEntry &entry) {
                                                    return entry.slotIndex == slotIndex;
                                                });
        if (entryIterator == layout.entries.end())
            continue;

        slotComponent->setDragged(draggedSlotIndex.has_value() && slotIndex == *draggedSlotIndex);
        slotComponent->setAlpha(slotComponent->getDragged() ? 0.0f : 1.0f);

        // Keep the dragged child anchored while dragging. The floating preview is painted
        // separately, so moving the real event source during drag destabilizes mouse coordinates.
        if (!slotComponent->getDragged())
            slotComponent->setBounds(entryIterator->bounds.toNearestInt());
    }

    if (addButton.isVisible()) {
        const auto nextLaneIndex = juce::jlimit<size_t>(0, layout.laneBounds.size() - 1, visibleOrderedSlots.size());
        const auto addBounds = getLaneModuleBounds(layout.laneBounds[nextLaneIndex].toNearestInt(), theme);
        addButton.setBounds(addBounds);
    }
}

void SignalRackComponent::paintOverChildren(juce::Graphics &g) {
    if (!dragSession.isDragging() || draggedSnapshot.isNull())
        return;

    auto draggedBounds = dragSession.getDraggedBounds();
    const auto rackBounds = getRackContentBounds(getLocalBounds(), theme);
    draggedBounds.setY(static_cast<float>(rackBounds.getY()));
    draggedBounds.setHeight(static_cast<float>(rackBounds.getHeight()));

    g.setOpacity(1.0f);
    g.drawImage(draggedSnapshot, draggedBounds);
}

void SignalRackComponent::refreshFromState(const bool force) {
    const auto &signalSlots = currentSnapshot.signalSlots;
    const auto sidechainAvailable = currentSnapshot.sidechainAvailable;
    const auto displayOrder = dragSession.getDisplayOrder(currentSnapshot.slotOrder);
    const auto draggedSlotIndex = dragSession.getDraggedSlotIndex();

    if (!force
        && lastSignalSlots.has_value() && *lastSignalSlots == signalSlots
        && lastSidechainAvailable.has_value() && *lastSidechainAvailable == sidechainAvailable
        && lastDisplayOrder.has_value() && *lastDisplayOrder == displayOrder
        && lastDraggedSlotIndex == draggedSlotIndex)
        return;

    lastSignalSlots = signalSlots;
    lastSidechainAvailable = sidechainAvailable;
    lastDisplayOrder = displayOrder;
    lastDraggedSlotIndex = draggedSlotIndex;

    std::vector<int> usedColours;
    std::vector<Ui::SignalSlotKey> usedSignalConfigs;
    usedColours.reserve(signalSlots.size());
    usedSignalConfigs.reserve(signalSlots.size());

    for (const auto &slot: signalSlots) {
        if (slot.configuration.enabled) {
            usedColours.push_back(slot.colourIndex);
            usedSignalConfigs.push_back(Ui::makeSignalSlotKey(slot.configuration.source, slot.configuration.mode));
        }
    }

    for (size_t slotIndex = 0; slotIndex < slotComponents.size(); ++slotIndex) {
        auto &component = slotComponents[slotIndex];
        component->setSidechainAvailable(sidechainAvailable);

        if (slotIndex >= signalSlots.size() || !signalSlots[slotIndex].configuration.enabled) {
            component->setVisible(false);
            component->setDragged(false);
            continue;
        }

        component->setVisible(true);
        component->setSlot(slotIndex, signalSlots[slotIndex], usedColours, usedSignalConfigs);
    }

    const auto activeCount = static_cast<int>(std::count_if(signalSlots.begin(), signalSlots.end(),
                                                            [](const Ui::SignalSlotState &slot) {
                                                                return slot.configuration.enabled;
                                                            }));
    addButton.setVisible(activeCount < static_cast<int>(Shared::maxSignalSlots));
    resized();
    repaint();
}

void SignalRackComponent::analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) {
    currentSnapshot = snapshot;
    refreshFromState();
}

void SignalRackComponent::addSignal() {
    const auto &signalSlots = currentSnapshot.signalSlots;
    const auto currentSlotOrder = currentSnapshot.slotOrder;

    const auto freeSlotIndex = Ui::findFreeSignalSlot(signalSlots);
    if (!freeSlotIndex.has_value())
        return;

    Ui::SignalSlotState slotState;
    slotState.configuration = Ui::chooseDefaultSignalConfiguration(signalSlots, currentSnapshot.sidechainAvailable);
    slotState.visible = true;
    slotState.frozen = false;
    slotState.colourIndex = Ui::chooseDefaultSignalColourIndex(signalSlots);
    slotState.opacity = Ui::defaultSignalOpacity;

    const auto updatedSlotOrder = Ui::appendSlotToEnd(currentSlotOrder, *freeSlotIndex);
    settingsActions.addSignalSlot(*freeSlotIndex, slotState, updatedSlotOrder);
}

std::vector<size_t> SignalRackComponent::getVisibleOrderedSlots(const Shared::SignalSlotOrder &slotOrder) const {
    return slotOrderModel.getVisibleOrderedSlots(currentSnapshot.signalSlots, slotOrder);
}

SignalRackLayout SignalRackComponent::buildLayout(const std::vector<size_t> &visibleOrderedSlots) const {
    return layoutEngine.build(getRackContentBounds(getLocalBounds(), theme).toFloat(),
                              makeItemSpecs(visibleOrderedSlots),
                              static_cast<float>(theme.metrics.sectionDivider.thickness));
}

std::vector<SignalRackItemSpec> SignalRackComponent::makeItemSpecs(
    const std::vector<size_t> &visibleOrderedSlots) const {
    std::vector<SignalRackItemSpec> itemSpecs;
    itemSpecs.reserve(visibleOrderedSlots.size());

    for (const auto slotIndex: visibleOrderedSlots) {
        if (slotIndex >= slotComponents.size())
            continue;

        SignalRackItemSpec itemSpec;
        itemSpec.slotIndex = slotIndex;
        itemSpecs.push_back(itemSpec);
    }

    return itemSpecs;
}

SignalSlotComponent *SignalRackComponent::findComponentForSlot(const size_t slotIndex) const {
    if (slotIndex >= slotComponents.size())
        return nullptr;

    return slotComponents[slotIndex].get();
}
