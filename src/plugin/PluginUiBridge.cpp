#include "PluginUiBridge.h"

PluginUiBridge::PluginUiBridge(PluginParameters::Access &parameterAccessToUse,
                               SignalSlotOrderState &signalSlotOrderStateToUse,
                               PresetSession &presetSessionToUse,
                               std::function<bool()> isSidechainAvailableToUse)
    : parameterAccess(parameterAccessToUse),
      signalSlotOrderState(signalSlotOrderStateToUse),
      presetSession(presetSessionToUse),
      isSidechainAvailable(std::move(isSidechainAvailableToUse)) {
}

PluginUiBridge::~PluginUiBridge() {
    cancelPendingUpdate();
}

void PluginUiBridge::requestUiRefresh() {
    if (const auto *messageManager = juce::MessageManager::getInstanceWithoutCreating();
        messageManager != nullptr && messageManager->isThisTheMessageThread()) {
        publishAllSnapshots();
        return;
    }

    triggerAsyncUpdate();
}

Ui::AnalyzerUiSnapshot PluginUiBridge::getAnalyzerUiSnapshot() const {
    Ui::AnalyzerUiSnapshot state;
    state.signalSlots = parameterAccess.readUiSlots();
    state.slotOrder = signalSlotOrderState.getOrder();
    state.meterSettings = parameterAccess.readMeterSettings();
    state.bandMode = parameterAccess.readBandMode();
    state.frozen = parameterAccess.readFreeze();
    state.sidechainAvailable = isSidechainAvailable();
    state.gridMinDb = parameterAccess.readGridMinDb();
    state.gridMaxDb = parameterAccess.readGridMaxDb();
    state.gridStepDb = parameterAccess.readGridStepDb();
    state.useCustomFrequencyRange = parameterAccess.readUseCustomFrequencyRange();
    state.visibleMinFrequencyHz = parameterAccess.readVisibleMinFrequencyHz();
    state.visibleMaxFrequencyHz = parameterAccess.readVisibleMaxFrequencyHz();
    return state;
}

void PluginUiBridge::addAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) {
    uiSnapshotListeners.add(&listener);
}

void PluginUiBridge::removeAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) {
    uiSnapshotListeners.remove(&listener);
}

Ui::EditorPresentationState PluginUiBridge::getEditorPresentationState() const {
    Ui::EditorPresentationState state;
    state.scale = parameterAccess.readUiScalePreset();
    return state;
}

void PluginUiBridge::addEditorPresentationStateListener(EditorPresentationStateSource::Listener &listener) {
    editorPresentationStateListeners.add(&listener);
}

void PluginUiBridge::removeEditorPresentationStateListener(EditorPresentationStateSource::Listener &listener) {
    editorPresentationStateListeners.remove(&listener);
}

Ui::Presets::PresetUiSnapshot PluginUiBridge::getPresetUiSnapshot() const {
    return presetSession.getSnapshot();
}

void PluginUiBridge::addPresetUiSnapshotListener(PresetUiSnapshotSource::Listener &listener) {
    presetUiSnapshotListeners.add(&listener);
}

void PluginUiBridge::removePresetUiSnapshotListener(PresetUiSnapshotSource::Listener &listener) {
    presetUiSnapshotListeners.remove(&listener);
}

void PluginUiBridge::setBandMode(const Analyzer::BandMode bandMode) {
    parameterAccess.writeBandMode(bandMode);
    requestUiRefresh();
}

void PluginUiBridge::setFreezeEnabled(const bool isFrozen) {
    parameterAccess.writeFreeze(isFrozen);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotEnabled(const size_t slotIndex, const bool isEnabled) {
    parameterAccess.writeSlotEnabled(slotIndex, isEnabled);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotVisible(const size_t slotIndex, const bool isVisible) {
    parameterAccess.writeSlotVisible(slotIndex, isVisible);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotFrozen(const size_t slotIndex, const bool isFrozen) {
    parameterAccess.writeSlotFrozen(slotIndex, isFrozen);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotSolo(const size_t slotIndex, const bool isSolo) {
    parameterAccess.writeSlotSolo(slotIndex, isSolo);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotSource(const size_t slotIndex, const Analyzer::SignalSource source) {
    parameterAccess.writeSlotSignal(slotIndex, source, parameterAccess.readUiSlot(slotIndex).configuration.mode);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotMode(const size_t slotIndex, const Analyzer::SignalMode mode) {
    parameterAccess.writeSlotSignal(slotIndex, parameterAccess.readUiSlot(slotIndex).configuration.source, mode);
    requestUiRefresh();
}

void PluginUiBridge::removeSignalSlot(const size_t slotIndex) {
    auto state = parameterAccess.readUiSlot(slotIndex);
    state.configuration.enabled = false;
    state.visible = false;
    state.frozen = false;
    parameterAccess.writeSlotState(slotIndex, state);
    requestUiRefresh();
}

void PluginUiBridge::addSignalSlot(const size_t slotIndex,
                                   const Ui::SignalSlotState &state,
                                   const Shared::SignalSlotOrder &slotOrder) {
    parameterAccess.writeSlotState(slotIndex, state);
    signalSlotOrderState.setOrder(slotOrder);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) {
    signalSlotOrderState.setOrder(slotOrder);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotColour(const size_t slotIndex, const int colourIndex) {
    parameterAccess.writeSlotColour(slotIndex, colourIndex);
    requestUiRefresh();
}

void PluginUiBridge::setSignalSlotOpacity(const size_t slotIndex, const float opacity) {
    parameterAccess.writeSlotOpacity(slotIndex, opacity);
    requestUiRefresh();
}

void PluginUiBridge::setShowPeakEnabled(const bool isEnabled) {
    parameterAccess.writeShowPeak(isEnabled);
    requestUiRefresh();
}

void PluginUiBridge::setShowRmsEnabled(const bool isEnabled) {
    parameterAccess.writeShowRms(isEnabled);
    requestUiRefresh();
}

void PluginUiBridge::setShowHoldEnabled(const bool isEnabled) {
    parameterAccess.writeShowHold(isEnabled);
    requestUiRefresh();
}

void PluginUiBridge::setHoldTimeMs(const float holdMs) {
    parameterAccess.writeHoldMs(holdMs);
    requestUiRefresh();
}

void PluginUiBridge::setRmsWindowMs(const float rmsWindowMs) {
    parameterAccess.writeRmsWindowMs(rmsWindowMs);
    requestUiRefresh();
}

void PluginUiBridge::setPeakDecayDbPerSecond(const float decayDbPerSecond) {
    parameterAccess.writePeakDecayDbPerSecond(decayDbPerSecond);
    requestUiRefresh();
}

void PluginUiBridge::setHoldDecayDbPerSecond(const float decayDbPerSecond) {
    parameterAccess.writeHoldDecayDbPerSecond(decayDbPerSecond);
    requestUiRefresh();
}

void PluginUiBridge::setGridMinDb(const float gridMinDb) {
    parameterAccess.writeGridMinDb(
        juce::jmin(gridMinDb, parameterAccess.readGridMaxDb() - Defaults::gridMinSpanDb));
    requestUiRefresh();
}

void PluginUiBridge::setGridMaxDb(const float gridMaxDb) {
    parameterAccess.writeGridMaxDb(
        juce::jmax(gridMaxDb, parameterAccess.readGridMinDb() + Defaults::gridMinSpanDb));
    requestUiRefresh();
}

void PluginUiBridge::setGridStepDb(const float gridStepDb) {
    parameterAccess.writeGridStepDb(gridStepDb);
    requestUiRefresh();
}

void PluginUiBridge::setCustomFrequencyRangeEnabled(const bool isEnabled) {
    parameterAccess.writeUseCustomFrequencyRange(isEnabled);
    requestUiRefresh();
}

void PluginUiBridge::setVisibleMinFrequencyHz(const float frequencyHz) {
    parameterAccess.writeVisibleMinFrequencyHz(
        juce::jmin(frequencyHz,
                   parameterAccess.readVisibleMaxFrequencyHz() / Defaults::visibleFrequencyMinSpanRatio));
    requestUiRefresh();
}

void PluginUiBridge::setVisibleMaxFrequencyHz(const float frequencyHz) {
    parameterAccess.writeVisibleMaxFrequencyHz(
        juce::jmax(frequencyHz,
                   parameterAccess.readVisibleMinFrequencyHz() * Defaults::visibleFrequencyMinSpanRatio));
    requestUiRefresh();
}

void PluginUiBridge::setUiScalePreset(const Ui::UiScalePreset preset) {
    parameterAccess.writeUiScalePreset(preset);
    requestUiRefresh();
}

Ui::Presets::PresetActionResult PluginUiBridge::loadPreset(const Ui::Presets::PresetId &presetId) {
    return finishPresetLoadAction(presetSession.loadPreset(presetId));
}

Ui::Presets::PresetActionResult PluginUiBridge::loadPreviousPreset() {
    return finishPresetLoadAction(presetSession.loadPreviousPreset());
}

Ui::Presets::PresetActionResult PluginUiBridge::loadNextPreset() {
    return finishPresetLoadAction(presetSession.loadNextPreset());
}

Ui::Presets::PresetActionResult PluginUiBridge::resetCurrentPreset() {
    return finishPresetLoadAction(presetSession.resetCurrentPreset());
}

Ui::Presets::PresetActionResult PluginUiBridge::savePresetAs(const juce::String &name) {
    const auto result = presetSession.savePresetAs(name);
    publishPresetUiSnapshot();
    return result;
}

Ui::Presets::PresetActionResult PluginUiBridge::overwritePreset(const Ui::Presets::PresetId &presetId,
                                                                const juce::String &name) {
    const auto result = presetSession.overwritePreset(presetId, name);
    publishPresetUiSnapshot();
    return result;
}

Ui::Presets::PresetActionResult PluginUiBridge::deletePreset(const Ui::Presets::PresetId &presetId) {
    const auto result = presetSession.deletePreset(presetId);
    publishPresetUiSnapshot();
    return result;
}

void PluginUiBridge::refreshPresetCatalog() {
    presetSession.refreshCatalog();
    publishPresetUiSnapshot();
}

void PluginUiBridge::processorPresetStateChanged() {
    presetSession.markCurrentStateDirty();
}

void PluginUiBridge::processorUiRefreshRequested() {
    requestUiRefresh();
}

void PluginUiBridge::handleAsyncUpdate() {
    publishAllSnapshots();
}

void PluginUiBridge::publishAllSnapshots() {
    publishAnalyzerUiSnapshot();
    publishEditorPresentationState();
    publishPresetUiSnapshot();
}

void PluginUiBridge::publishAnalyzerUiSnapshot() {
    const auto snapshot = getAnalyzerUiSnapshot();
    if (lastPublishedUiSnapshot.has_value() && *lastPublishedUiSnapshot == snapshot)
        return;

    lastPublishedUiSnapshot = snapshot;
    uiSnapshotListeners.call([&snapshot](AnalyzerUiSnapshotSource::Listener &listener) {
        listener.analyzerUiSnapshotChanged(snapshot);
    });
}

void PluginUiBridge::publishEditorPresentationState() {
    const auto state = getEditorPresentationState();
    if (lastPublishedEditorPresentationState.has_value() && *lastPublishedEditorPresentationState == state)
        return;

    lastPublishedEditorPresentationState = state;
    editorPresentationStateListeners.call([&state](EditorPresentationStateSource::Listener &listener) {
        listener.editorPresentationStateChanged(state);
    });
}

void PluginUiBridge::publishPresetUiSnapshot() {
    const auto snapshot = getPresetUiSnapshot();
    if (lastPublishedPresetUiSnapshot.has_value() && *lastPublishedPresetUiSnapshot == snapshot)
        return;

    lastPublishedPresetUiSnapshot = snapshot;
    presetUiSnapshotListeners.call([&snapshot](PresetUiSnapshotSource::Listener &listener) {
        listener.presetUiSnapshotChanged(snapshot);
    });
}

Ui::Presets::PresetActionResult PluginUiBridge::finishPresetLoadAction(const Ui::Presets::PresetActionResult &result) {
    if (!result.succeeded) {
        publishPresetUiSnapshot();
        return result;
    }

    requestUiRefresh();
    return result;
}
