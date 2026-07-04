#pragma once

#include <atomic>
#include <optional>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "FactoryPresetRepository.h"
#include "PluginStateSerializer.h"
#include "UserPresetStore.h"
#include "ui/presets/state/PresetUiState.h"

class PresetSession final {
public:
    PresetSession(juce::AudioProcessorValueTreeState& parametersToUse,
                  SignalSlotOrderState& signalSlotOrderStateToUse,
                  PluginStateSerializer& stateSerializerToUse,
                  FactoryPresetRepository& factoryRepositoryToUse,
                  UserPresetStore& userPresetStoreToUse);

    [[nodiscard]] const Ui::Presets::PresetUiSnapshot& getSnapshot() const;
    [[nodiscard]] std::optional<Ui::Presets::PresetId> getSelectedPresetId() const noexcept;

    [[nodiscard]] Ui::Presets::PresetActionResult loadPreset(const Ui::Presets::PresetId& presetId);
    [[nodiscard]] Ui::Presets::PresetActionResult loadPreviousPreset();
    [[nodiscard]] Ui::Presets::PresetActionResult loadNextPreset();
    [[nodiscard]] Ui::Presets::PresetActionResult resetCurrentPreset();
    [[nodiscard]] Ui::Presets::PresetActionResult savePresetAs(const juce::String& name);
    [[nodiscard]] Ui::Presets::PresetActionResult overwritePreset(const Ui::Presets::PresetId& presetId,
                                                                  const juce::String& name);
    [[nodiscard]] Ui::Presets::PresetActionResult deletePreset(const Ui::Presets::PresetId& presetId);
    void refreshCatalog();
    void restoreSelection(const std::optional<Ui::Presets::PresetId>& presetId);
    void markCurrentStateDirty() noexcept;

private:
    [[nodiscard]] const PluginPresets::PluginStateSnapshot& captureCurrentState() const;
    [[nodiscard]] const std::vector<PluginPresets::PresetDocument>& getMergedCatalog() const;
    void invalidateMergedCatalog();
    void rebuildMergedCatalog() const;
    void adoptDocumentState(const PluginPresets::PresetDocument& document);
    [[nodiscard]] const PluginPresets::PresetDocument* getSelectedDocument(
        const std::vector<PluginPresets::PresetDocument>& catalog) const;
    void resolveSelectionFromCurrentState();
    void markSnapshotDirty() noexcept;
    [[nodiscard]] Ui::Presets::PresetUiSnapshot buildSnapshot(
        const std::vector<PluginPresets::PresetDocument>& catalog,
        const PluginPresets::PluginStateSnapshot& currentState) const;

    juce::AudioProcessorValueTreeState& parameters;
    SignalSlotOrderState& signalSlotOrderState;
    PluginStateSerializer& stateSerializer;
    UserPresetStore& userPresetStore;
    const std::vector<PluginPresets::PresetDocument> factoryCatalog;
    mutable std::vector<PluginPresets::PresetDocument> mergedCatalogCache;
    mutable bool mergedCatalogDirty = true;
    mutable std::optional<PluginPresets::PluginStateSnapshot> currentStateSnapshotCache;
    mutable std::atomic<bool> currentStateSnapshotDirty { true };
    mutable std::optional<Ui::Presets::PresetUiSnapshot> snapshotCache;
    mutable std::atomic<bool> snapshotDirty { true };
    std::optional<Ui::Presets::PresetId> selectedPresetId;
};
