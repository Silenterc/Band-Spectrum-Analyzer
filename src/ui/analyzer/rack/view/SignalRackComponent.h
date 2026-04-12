#pragma once

#include <array>
#include <memory>
#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/state/SignalSlotUiState.h"
#include "ui/contracts/AnalyzerSettingsActions.h"
#include "ui/contracts/AnalyzerUiSnapshotSource.h"
#include "ui/widgets/SectionDividerComponent.h"
#include "ui/theme/UiTheme.h"
#include "SignalSlotActionButton.h"
#include "SignalSlotComponent.h"
#include "ui/analyzer/rack/interaction/SignalRackDragSession.h"
#include "ui/analyzer/rack/interaction/SignalRackLayoutEngine.h"
#include "ui/analyzer/rack/model/SignalSlotOrderModel.h"
#include "ui/analyzer/rack/model/SignalSlotOptions.h"

class SignalRackComponent final : public juce::Component,
                                  private AnalyzerUiSnapshotSource::Listener,
                                  private SignalSlotComponent::Listener {
public:
    SignalRackComponent(AnalyzerUiSnapshotSource &uiSnapshotSourceToUse,
                        AnalyzerSettingsActions &settingsActionsToUse,
                        const Ui::Theme &themeToUse);
    ~SignalRackComponent() override;

    void resized() override;
    void paintOverChildren(juce::Graphics &g) override;

private:
    void signalSlotSourceSelected(size_t slotIndex, Analyzer::SignalSource source) override;
    void signalSlotModeSelected(size_t slotIndex, Analyzer::SignalMode mode) override;
    void signalSlotColourSelected(size_t slotIndex, int colourIndex) override;
    void signalSlotVisibilityChanged(size_t slotIndex, bool isVisible) override;
    void signalSlotFrozenChanged(size_t slotIndex, bool isFrozen) override;
    void signalSlotRemoveRequested(size_t slotIndex) override;
    void signalSlotOpacityChanged(size_t slotIndex, float opacity) override;
    void signalSlotReorderDragStarted(size_t slotIndex, float startMouseX) override;
    void signalSlotReorderDragged(float xPosition) override;
    void signalSlotReorderDragEnded(float xPosition) override;
    void refreshFromState(bool force = false);
    void analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) override;
    void addSignal();
    SignalRackLayout buildLayout(const std::vector<size_t> &visibleOrderedSlots) const;
    std::vector<size_t> getVisibleOrderedSlots(const Shared::SignalSlotOrder &slotOrder) const;
    std::vector<SignalRackItemSpec> makeItemSpecs(const std::vector<size_t> &visibleOrderedSlots) const;
    SignalSlotComponent *findComponentForSlot(size_t slotIndex) const;

    AnalyzerUiSnapshotSource &uiSnapshotSource;
    AnalyzerSettingsActions &settingsActions;
    const Ui::Theme &theme;
    SignalSlotOrderModel slotOrderModel;
    SignalRackLayoutEngine layoutEngine;
    SignalRackDragSession dragSession;
    std::array<std::unique_ptr<SectionDividerComponent>, Shared::maxSignalSlots - 1> slotDividers;
    std::array<std::unique_ptr<SignalSlotComponent>, Shared::maxSignalSlots> slotComponents;
    juce::Image draggedSnapshot;
    SignalSlotActionButton addButton;
    std::optional<std::array<Ui::SignalSlotState, Shared::maxSignalSlots>> lastSignalSlots;
    std::optional<Shared::SignalSlotOrder> lastDisplayOrder;
    std::optional<bool> lastSidechainAvailable;
    std::optional<size_t> lastDraggedSlotIndex;
    Ui::AnalyzerUiSnapshot currentSnapshot;
};
