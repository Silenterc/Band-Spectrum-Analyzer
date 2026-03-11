#include "SignalRackComponent.h"

#include <algorithm>

namespace {
    Shared::SignalSlotOrder appendSlotToEnd(const Shared::SignalSlotOrder &slotOrder, const size_t slotIndex) {
        Shared::SignalSlotOrder reordered = slotOrder;

        auto writeIt = std::remove(reordered.begin(), reordered.end(), slotIndex);
        std::fill(writeIt, reordered.end(), slotIndex);
        reordered[reordered.size() - 1] = slotIndex;

        size_t fillIndex = 0;
        for (auto readIt = reordered.begin(); readIt != reordered.end(); ++readIt) {
            if (*readIt == slotIndex && readIt != reordered.end() - 1)
                continue;

            reordered[fillIndex++] = *readIt;
        }

        return reordered;
    }
}

SignalRackComponent::SignalRackComponent(AnalyzerUiStateSource &uiStateSourceToUse,
                                         AnalyzerSettingsActions &settingsActionsToUse,
                                         const Ui::Theme &themeToUse)
    : uiStateSource(uiStateSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse),
      flatButtonLookAndFeel(themeToUse) {
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
        slotComponent->onRemoveClicked = [this](const size_t slotIndex) {
            updateLocalSlot(slotIndex, [](Ui::SignalSlotState &slotState) {
                slotState.configuration.enabled = false;
            });
            settingsActions.setSignalSlotEnabled(slotIndex, false);
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
            const auto layout = layoutEngine.build(getLocalBounds().reduced(0, static_cast<int>(theme.metrics.rack.verticalInset)).toFloat(),
                                                   makeItemSpecs(visibleOrderedSlots), theme.metrics.rack.itemGap);
            dragSession.begin(slotIndex, persistedOrder, visibleOrderedSlots, layout, startMouseX);
            if (auto *draggedComponent = findComponentForSlot(slotIndex))
                draggedSnapshot = draggedComponent->createComponentSnapshot(draggedComponent->getLocalBounds());
            refreshFromState(true);
        };
        slotComponent->onReorderDragged = [this](const size_t slotIndex, const float xPosition) {
            juce::ignoreUnused(slotIndex);
            const auto previewOrder = dragSession.getDisplayOrder(currentState.slotOrder);
            const auto visibleOrderedSlots = getVisibleOrderedSlots(previewOrder);
            const auto layout = layoutEngine.build(getLocalBounds().reduced(0, static_cast<int>(theme.metrics.rack.verticalInset)).toFloat(),
                                                   makeItemSpecs(visibleOrderedSlots), theme.metrics.rack.itemGap);
            dragSession.update(xPosition, layout);
            refreshFromState(true);
        };
        slotComponent->onReorderDragEnded = [this](const size_t slotIndex, const float xPosition) {
            juce::ignoreUnused(slotIndex);
            const auto previewOrder = dragSession.getDisplayOrder(currentState.slotOrder);
            const auto visibleOrderedSlots = getVisibleOrderedSlots(previewOrder);
            const auto layout = layoutEngine.build(getLocalBounds().reduced(0, static_cast<int>(theme.metrics.rack.verticalInset)).toFloat(),
                                                   makeItemSpecs(visibleOrderedSlots), theme.metrics.rack.itemGap);
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
    const auto rackBounds = getLocalBounds().reduced(0, static_cast<int>(theme.metrics.rack.verticalInset));
    const auto displayOrder = dragSession.getDisplayOrder(currentState.slotOrder);
    const auto visibleOrderedSlots = getVisibleOrderedSlots(displayOrder);
    const auto layout = layoutEngine.build(rackBounds.toFloat(), makeItemSpecs(visibleOrderedSlots), theme.metrics.rack.itemGap);
    const auto draggedSlotIndex = dragSession.getDraggedSlotIndex();

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
        const auto addX = layout.entries.empty()
                              ? rackBounds.getX()
                              : static_cast<int>(layout.activeSpan.getRight()) + static_cast<int>(theme.metrics.rack.itemGap);
        const auto addSize = rackBounds.getHeight();
        addButton.setBounds(addX, rackBounds.getY(), addSize, addSize);
    }
}

void SignalRackComponent::paintOverChildren(juce::Graphics &g) {
    if (!dragSession.isDragging() || draggedSnapshot.isNull())
        return;

    auto draggedBounds = dragSession.getDraggedBounds();
    const auto rackBounds = getLocalBounds().reduced(0, static_cast<int>(theme.metrics.rack.verticalInset));
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
    std::vector<std::pair<Analyzer::SignalSource, Analyzer::SignalMode>> usedSignalConfigs;
    usedColours.reserve(signalSlots.size());
    usedSignalConfigs.reserve(signalSlots.size());

    for (const auto &slot: signalSlots) {
        if (slot.configuration.enabled) {
            usedColours.push_back(slot.colourIndex);
            usedSignalConfigs.emplace_back(slot.configuration.source, slot.configuration.mode);
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

    std::optional<size_t> freeSlotIndex;
    for (size_t slotIndex = 0; slotIndex < signalSlots.size(); ++slotIndex) {
        if (!signalSlots[slotIndex].configuration.enabled) {
            freeSlotIndex = slotIndex;
            break;
        }
    }

    if (!freeSlotIndex.has_value())
        return;

    const auto sidechainAvailable = currentState.sidechainAvailable;
    Analyzer::SignalSource defaultSource = Analyzer::SignalSource::main;
    Analyzer::SignalMode defaultMode = Analyzer::SignalMode::mid;

    constexpr std::array<std::pair<Analyzer::SignalSource, Analyzer::SignalMode>, 6> preferredSignals{{
        {Analyzer::SignalSource::main, Analyzer::SignalMode::mid},
        {Analyzer::SignalSource::main, Analyzer::SignalMode::side},
        {Analyzer::SignalSource::main, Analyzer::SignalMode::stereo},
        {Analyzer::SignalSource::sidechain, Analyzer::SignalMode::mid},
        {Analyzer::SignalSource::sidechain, Analyzer::SignalMode::side},
        {Analyzer::SignalSource::sidechain, Analyzer::SignalMode::stereo}
    }};

    for (const auto &[source, mode]: preferredSignals) {
        if (source == Analyzer::SignalSource::sidechain && !sidechainAvailable)
            continue;

        if (!isSignalConfigUsed(signalSlots, source, mode)) {
            defaultSource = source;
            defaultMode = mode;
            break;
        }
    }

    int defaultColour = 0;
    for (int colourIndex = 0; colourIndex < Ui::signalPresetCount; ++colourIndex) {
        const auto colourInUse = std::any_of(signalSlots.begin(), signalSlots.end(),
                                             [colourIndex](const Ui::SignalSlotState &slot) {
                                                 return slot.configuration.enabled && slot.colourIndex == colourIndex;
                                             });
        if (!colourInUse) {
            defaultColour = colourIndex;
            break;
        }
    }

    Ui::SignalSlotState slotState;
    slotState.configuration.enabled = true;
    slotState.configuration.source = defaultSource;
    slotState.configuration.mode = defaultMode;
    slotState.visible = true;
    slotState.colourIndex = defaultColour;
    slotState.opacity = Ui::defaultSignalOpacity;

    beginOptimisticUpdate();
    const auto updatedSlotOrder = appendSlotToEnd(currentSlotOrder, *freeSlotIndex);
    setLocalSlotState(*freeSlotIndex, slotState);
    setLocalSlotOrder(updatedSlotOrder);
    settingsActions.applySignalSlotState(*freeSlotIndex, slotState);
    settingsActions.setSignalSlotOrder(updatedSlotOrder);
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

bool SignalRackComponent::isSignalConfigUsed(
    const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
    const Analyzer::SignalSource source,
    const Analyzer::SignalMode mode) {
    return std::any_of(signalSlots.begin(), signalSlots.end(),
                       [source, mode](const Ui::SignalSlotState &slot) {
                           return slot.configuration.enabled
                                  && slot.configuration.source == source
                                  && slot.configuration.mode == mode;
                       });
}

std::vector<size_t> SignalRackComponent::getVisibleOrderedSlots(const Shared::SignalSlotOrder &slotOrder) const {
    return slotOrderModel.getVisibleOrderedSlots(currentState.signalSlots, slotOrder);
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
        itemSpec.width = static_cast<float>(slotComponents[slotIndex]->getPreferredWidth());
        itemSpecs.push_back(itemSpec);
    }

    return itemSpecs;
}

SignalSlotComponent *SignalRackComponent::findComponentForSlot(const size_t slotIndex) const {
    if (slotIndex >= slotComponents.size())
        return nullptr;

    return slotComponents[slotIndex].get();
}
