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

SignalRackComponent::SignalRackComponent(AnalyzerUiStateSource &uiStateSourceToUse,
                                         AnalyzerSettingsActions &settingsActionsToUse,
                                         const Ui::Theme &themeToUse)
    : uiStateSource(uiStateSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse),
      flatButtonLookAndFeel(themeToUse) {
    for (auto &slotDivider: slotDividers) {
        slotDivider = std::make_unique<SectionDividerComponent>(theme, SectionDividerComponent::Orientation::vertical);
        addAndMakeVisible(*slotDivider);
    }

    for (auto &slotComponent: slotComponents) {
        slotComponent = std::make_unique<SignalSlotComponent>(theme);
        addAndMakeVisible(*slotComponent);
        slotComponent->onSignalSelected = [this](const size_t slotIndex, const Analyzer::SignalSource source,
                                                 const Analyzer::SignalMode mode) {
            updateLocalSlot(slotIndex, [source, mode](Ui::SignalSlotState &slotState) {
                slotState.configuration.source = source;
                slotState.configuration.mode = mode;
            });
            settingsActions.setSignalSlotSignal(slotIndex, source, mode);
        };
        slotComponent->onColourSelected = [this](const size_t slotIndex, const int colourIndex) {
            updateLocalSlot(slotIndex, [colourIndex](Ui::SignalSlotState &slotState) {
                slotState.colourIndex = colourIndex;
            });
            settingsActions.setSignalSlotColour(slotIndex, colourIndex);
        };
        slotComponent->onVisibilityChanged = [this](const size_t slotIndex, const bool isVisible) {
            updateLocalSlot(slotIndex, [isVisible](Ui::SignalSlotState &slotState) {
                slotState.visible = isVisible;
            });
            settingsActions.setSignalSlotVisible(slotIndex, isVisible);
        };
        slotComponent->onFrozenChanged = [this](const size_t slotIndex, const bool isFrozen) {
            updateLocalSlot(slotIndex, [isFrozen](Ui::SignalSlotState &slotState) {
                slotState.frozen = isFrozen;
            });
            settingsActions.setSignalSlotFrozen(slotIndex, isFrozen);
        };
        slotComponent->onRemoveClicked = [this](const size_t slotIndex) {
            updateLocalSlot(slotIndex, [](Ui::SignalSlotState &slotState) {
                slotState.configuration.enabled = false;
                slotState.frozen = false;
            });
            settingsActions.removeSignalSlot(slotIndex);
        };
        slotComponent->onOpacityChanged = [this](const size_t slotIndex, const float opacity) {
            updateLocalSlot(slotIndex, [opacity](Ui::SignalSlotState &slotState) {
                slotState.opacity = opacity;
            });
            settingsActions.setSignalSlotOpacity(slotIndex, opacity);
        };
        slotComponent->onReorderDragStarted = [this](const size_t slotIndex, const float startMouseX) {
            const auto persistedOrder = currentState.slotOrder;
            const auto visibleOrderedSlots = getVisibleOrderedSlots(persistedOrder);
            const auto layout = buildLayout(visibleOrderedSlots);
            dragSession.begin(slotIndex, persistedOrder, visibleOrderedSlots, layout, startMouseX);
            if (auto *draggedComponent = findComponentForSlot(slotIndex))
                draggedSnapshot = draggedComponent->createComponentSnapshot(draggedComponent->getLocalBounds());
            refreshFromState(true);
        };
        slotComponent->onReorderDragged = [this](const size_t slotIndex, const float xPosition) {
            juce::ignoreUnused(slotIndex);
            const auto previewOrder = dragSession.getDisplayOrder(currentState.slotOrder);
            const auto visibleOrderedSlots = getVisibleOrderedSlots(previewOrder);
            const auto layout = buildLayout(visibleOrderedSlots);
            dragSession.update(xPosition, layout);
            refreshFromState(true);
        };
        slotComponent->onReorderDragEnded = [this](const size_t slotIndex, const float xPosition) {
            juce::ignoreUnused(slotIndex);
            const auto previewOrder = dragSession.getDisplayOrder(currentState.slotOrder);
            const auto visibleOrderedSlots = getVisibleOrderedSlots(previewOrder);
            const auto layout = buildLayout(visibleOrderedSlots);
            dragSession.update(xPosition, layout);
            const auto reorderedSlots = dragSession.finish();
            if (reorderedSlots.has_value()) {
                setLocalSlotOrder(*reorderedSlots);
                settingsActions.setSignalSlotOrder(*reorderedSlots);
            }
            draggedSnapshot = {};
            refreshFromState(true);
        };
    }

    addAndMakeVisible(addButton);
    addButton.setLookAndFeel(&flatButtonLookAndFeel);
    addButton.onClick = [this] { addSignal(); };
    addButton.setTooltip("Add signal");
    uiStateSource.addAnalyzerUiStateListener(*this);
    analyzerUiStateChanged(uiStateSource.getAnalyzerUiState());
}

SignalRackComponent::~SignalRackComponent() {
    addButton.setLookAndFeel(nullptr);
    uiStateSource.removeAnalyzerUiStateListener(*this);
}

void SignalRackComponent::resized() {
    const auto displayOrder = dragSession.getDisplayOrder(currentState.slotOrder);
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
    const auto &signalSlots = currentState.signalSlots;
    const auto sidechainAvailable = currentState.sidechainAvailable;
    const auto displayOrder = dragSession.getDisplayOrder(currentState.slotOrder);
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
    addButton.setColour(juce::TextButton::buttonColourId, theme.controlSurface);
    addButton.setColour(juce::TextButton::buttonOnColourId, theme.controlSurfaceHover);
    addButton.setColour(juce::TextButton::textColourOffId, theme.controlText);
    resized();
    repaint();
}

void SignalRackComponent::analyzerUiStateChanged(const Ui::AnalyzerUiState &state) {
    if (optimisticUpdateDepth > 0)
        return;

    currentState = state;
    refreshFromState();
}

void SignalRackComponent::addSignal() {
    const auto &signalSlots = currentState.signalSlots;
    const auto currentSlotOrder = currentState.slotOrder;

    const auto freeSlotIndex = Ui::findFreeSignalSlot(signalSlots);
    if (!freeSlotIndex.has_value())
        return;

    Ui::SignalSlotState slotState;
    slotState.configuration = Ui::chooseDefaultSignalConfiguration(signalSlots, currentState.sidechainAvailable);
    slotState.visible = true;
    slotState.frozen = false;
    slotState.colourIndex = Ui::chooseDefaultSignalColourIndex(signalSlots);
    slotState.opacity = Ui::defaultSignalOpacity;

    beginOptimisticUpdate();
    const auto updatedSlotOrder = Ui::appendSlotToEnd(currentSlotOrder, *freeSlotIndex);
    setLocalSlotState(*freeSlotIndex, slotState);
    setLocalSlotOrder(updatedSlotOrder);
    settingsActions.addSignalSlot(*freeSlotIndex, slotState, updatedSlotOrder);
    endOptimisticUpdate();
}

void SignalRackComponent::setLocalSlotState(const size_t slotIndex, const Ui::SignalSlotState &slotState) {
    if (slotIndex >= currentState.signalSlots.size())
        return;

    currentState.signalSlots[slotIndex] = slotState;
    refreshFromState(true);
}

void SignalRackComponent::updateLocalSlot(
    const size_t slotIndex,
    const std::function<void(Ui::SignalSlotState &slotState)> &update) {
    if (slotIndex >= currentState.signalSlots.size() || update == nullptr)
        return;

    update(currentState.signalSlots[slotIndex]);
    refreshFromState(true);
}

void SignalRackComponent::setLocalSlotOrder(const Shared::SignalSlotOrder &slotOrder) {
    currentState.slotOrder = slotOrder;
    refreshFromState(true);
}

void SignalRackComponent::beginOptimisticUpdate() {
    ++optimisticUpdateDepth;
}

void SignalRackComponent::endOptimisticUpdate() {
    if (optimisticUpdateDepth <= 0) {
        optimisticUpdateDepth = 0;
        return;
    }

    --optimisticUpdateDepth;
    if (optimisticUpdateDepth == 0) {
        currentState = uiStateSource.getAnalyzerUiState();
        refreshFromState(true);
    }
}

std::vector<size_t> SignalRackComponent::getVisibleOrderedSlots(const Shared::SignalSlotOrder &slotOrder) const {
    return slotOrderModel.getVisibleOrderedSlots(currentState.signalSlots, slotOrder);
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
