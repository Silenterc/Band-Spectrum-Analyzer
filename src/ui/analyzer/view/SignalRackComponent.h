#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../SignalSlotUiState.h"
#include "../../AnalyzerSettingsActions.h"
#include "../../AnalyzerUiSnapshotSource.h"
#include "../../SectionDividerComponent.h"
#include "../../UiTheme.h"
#include "SignalSlotActionButton.h"
#include "SignalSlotComponent.h"
#include "../helpers/SignalRackDragSession.h"
#include "../helpers/SignalRackLayoutEngine.h"
#include "../model/SignalSlotOrderModel.h"
#include "../model/SignalSlotOptions.h"

class SignalRackComponent final : public juce::Component,
                                  private AnalyzerUiSnapshotSource::Listener {
public:
    SignalRackComponent(AnalyzerUiSnapshotSource &uiSnapshotSourceToUse,
                        AnalyzerSettingsActions &settingsActionsToUse,
                        const Ui::Theme &themeToUse);
    ~SignalRackComponent() override;

    void resized() override;
    void paintOverChildren(juce::Graphics &g) override;

private:
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
