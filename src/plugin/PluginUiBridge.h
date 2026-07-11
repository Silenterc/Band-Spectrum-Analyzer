#pragma once

#include <functional>
#include <optional>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../ui/analyzer/contracts/AnalyzerSettingsActions.h"
#include "../ui/analyzer/contracts/AnalyzerUiSnapshotSource.h"
#include "../ui/editor/contracts/EditorPresentationActions.h"
#include "../ui/editor/contracts/EditorPresentationStateSource.h"
#include "../ui/presets/contracts/PresetActions.h"
#include "../ui/presets/contracts/PresetUiSnapshotSource.h"
#include "ProcessorChangeTracker.h"
#include "parameters/ParameterAccess.h"
#include "presets/PresetSession.h"
#include "state/SignalSlotOrderState.h"

/**
 * Implements the UI-facing contracts on behalf of the processor: owns snapshot
 * publication, listener lists, dedup caches, and message-thread marshalling.
 */
class PluginUiBridge final : public AnalyzerSettingsActions,
                             public AnalyzerUiSnapshotSource,
                             public EditorPresentationActions,
                             public EditorPresentationStateSource,
                             public PresetActions,
                             public PresetUiSnapshotSource,
                             public ProcessorChangeTracker::Listener,
                             private juce::AsyncUpdater {
public:
    PluginUiBridge(PluginParameters::Access &parameterAccess,
                   SignalSlotOrderState &signalSlotOrderState,
                   PresetSession &presetSession,
                   std::function<bool()> isSidechainAvailable);
    ~PluginUiBridge() override;

    /** Publishes all UI snapshots, immediately on the message thread, otherwise async. */
    void requestUiRefresh();

    // AnalyzerUiSnapshotSource
    Ui::AnalyzerUiSnapshot getAnalyzerUiSnapshot() const override;
    void addAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) override;
    void removeAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) override;

    // EditorPresentationStateSource
    Ui::EditorPresentationState getEditorPresentationState() const override;
    void addEditorPresentationStateListener(EditorPresentationStateSource::Listener &listener) override;
    void removeEditorPresentationStateListener(EditorPresentationStateSource::Listener &listener) override;

    // PresetUiSnapshotSource
    Ui::Presets::PresetUiSnapshot getPresetUiSnapshot() const override;
    void addPresetUiSnapshotListener(PresetUiSnapshotSource::Listener &listener) override;
    void removePresetUiSnapshotListener(PresetUiSnapshotSource::Listener &listener) override;

    // AnalyzerSettingsActions
    void setBandMode(Analyzer::BandMode bandMode) override;
    void setFreezeEnabled(bool isFrozen) override;
    void setSignalSlotEnabled(size_t slotIndex, bool isEnabled) override;
    void setSignalSlotVisible(size_t slotIndex, bool isVisible) override;
    void setSignalSlotFrozen(size_t slotIndex, bool isFrozen) override;
    void setSignalSlotSolo(size_t slotIndex, bool isSolo) override;
    void setSignalSlotSource(size_t slotIndex, Analyzer::SignalSource source) override;
    void setSignalSlotMode(size_t slotIndex, Analyzer::SignalMode mode) override;
    void removeSignalSlot(size_t slotIndex) override;
    void addSignalSlot(size_t slotIndex,
                       const Ui::SignalSlotState &state,
                       const Shared::SignalSlotOrder &slotOrder) override;
    void setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) override;
    void setSignalSlotColour(size_t slotIndex, int colourIndex) override;
    void setSignalSlotOpacity(size_t slotIndex, float opacity) override;
    void setShowPeakEnabled(bool isEnabled) override;
    void setShowRmsEnabled(bool isEnabled) override;
    void setShowHoldEnabled(bool isEnabled) override;
    void setHoldTimeMs(float holdMs) override;
    void setRmsWindowMs(float rmsWindowMs) override;
    void setPeakDecayDbPerSecond(float decayDbPerSecond) override;
    void setHoldDecayDbPerSecond(float decayDbPerSecond) override;
    void setGridMinDb(float gridMinDb) override;
    void setGridMaxDb(float gridMaxDb) override;
    void setGridStepDb(float gridStepDb) override;
    void setCustomFrequencyRangeEnabled(bool isEnabled) override;
    void setVisibleMinFrequencyHz(float frequencyHz) override;
    void setVisibleMaxFrequencyHz(float frequencyHz) override;

    // EditorPresentationActions
    void setUiScalePreset(Ui::UiScalePreset preset) override;

    // PresetActions
    [[nodiscard]] Ui::Presets::PresetActionResult loadPreset(const Ui::Presets::PresetId &presetId) override;
    [[nodiscard]] Ui::Presets::PresetActionResult loadPreviousPreset() override;
    [[nodiscard]] Ui::Presets::PresetActionResult loadNextPreset() override;
    [[nodiscard]] Ui::Presets::PresetActionResult resetCurrentPreset() override;
    [[nodiscard]] Ui::Presets::PresetActionResult savePresetAs(const juce::String &name) override;
    [[nodiscard]] Ui::Presets::PresetActionResult overwritePreset(const Ui::Presets::PresetId &presetId,
                                                                  const juce::String &name) override;
    [[nodiscard]] Ui::Presets::PresetActionResult deletePreset(const Ui::Presets::PresetId &presetId) override;
    void refreshPresetCatalog() override;

    // ProcessorChangeTracker::Listener
    void processorPresetStateChanged() override;
    void processorUiRefreshRequested() override;

private:
    void handleAsyncUpdate() override;
    void publishAllSnapshots();
    void publishAnalyzerUiSnapshot();
    void publishEditorPresentationState();
    void publishPresetUiSnapshot();
    Ui::Presets::PresetActionResult finishPresetLoadAction(const Ui::Presets::PresetActionResult &result);

    PluginParameters::Access &parameterAccess;
    SignalSlotOrderState &signalSlotOrderState;
    PresetSession &presetSession;
    std::function<bool()> isSidechainAvailable;

    std::optional<Ui::AnalyzerUiSnapshot> lastPublishedUiSnapshot;
    std::optional<Ui::EditorPresentationState> lastPublishedEditorPresentationState;
    std::optional<Ui::Presets::PresetUiSnapshot> lastPublishedPresetUiSnapshot;
    juce::ListenerList<AnalyzerUiSnapshotSource::Listener> uiSnapshotListeners;
    juce::ListenerList<EditorPresentationStateSource::Listener> editorPresentationStateListeners;
    juce::ListenerList<PresetUiSnapshotSource::Listener> presetUiSnapshotListeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginUiBridge)
};
