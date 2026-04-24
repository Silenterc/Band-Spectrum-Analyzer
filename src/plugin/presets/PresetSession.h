#pragma once

#include <atomic>
#include <optional>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "FactoryPresetRepository.h"
#include "PluginStateSerializer.h"
#include "UserPresetStore.h"

class PresetSession final {
public:
    PresetSession(juce::AudioProcessorValueTreeState& parametersToUse,
                  SignalSlotOrderState& signalSlotOrderStateToUse,
                  PluginStateSerializer& stateSerializerToUse,
                  FactoryPresetRepository& factoryRepositoryToUse,
                  UserPresetStore& userPresetStoreToUse);

    [[nodiscard]] const PluginPresets::PresetUiSnapshot& getSnapshot() const;
    [[nodiscard]] std::optional<PluginPresets::PresetId> getSelectedPresetId() const noexcept;

    [[nodiscard]] PluginPresets::PresetActionResult loadPreset(const PluginPresets::PresetId& presetId);
    [[nodiscard]] PluginPresets::PresetActionResult loadPreviousPreset();
    [[nodiscard]] PluginPresets::PresetActionResult loadNextPreset();
    [[nodiscard]] PluginPresets::PresetActionResult resetCurrentPreset();
    [[nodiscard]] PluginPresets::PresetActionResult savePresetAs(const juce::String& name);
    [[nodiscard]] PluginPresets::PresetActionResult overwritePreset(const PluginPresets::PresetId& presetId,
                                                                    const juce::String& name);
    [[nodiscard]] PluginPresets::PresetActionResult deletePreset(const PluginPresets::PresetId& presetId);
    void refreshCatalog();
    void restoreSelection(const std::optional<PluginPresets::PresetId>& presetId);
    void markCurrentStateDirty() noexcept;

private:
    [[nodiscard]] const PluginPresets::PluginStateSnapshot& captureCurrentState() const;
    [[nodiscard]] const std::vector<PluginPresets::PresetDocument>& getMergedCatalog() const;
    void invalidateMergedCatalog();
    void rebuildMergedCatalog() const;
    [[nodiscard]] std::optional<PluginPresets::PresetDocument> findPresetById(
        const std::vector<PluginPresets::PresetDocument>& catalog,
        const PluginPresets::PresetId& presetId) const;
    [[nodiscard]] std::optional<size_t> findPresetIndex(
        const std::vector<PluginPresets::PresetDocument>& catalog,
        const PluginPresets::PresetId& presetId) const;
    [[nodiscard]] std::optional<PluginPresets::PresetDocument> findExistingUserPresetByName(
        const std::vector<PluginPresets::PresetDocument>& catalog,
        const juce::String& name) const;
    [[nodiscard]] std::optional<PluginPresets::PresetDocument> findFactoryPresetById(
        const std::vector<PluginPresets::PresetDocument>& catalog,
        const PluginPresets::PresetId& presetId) const;
    [[nodiscard]] std::optional<PluginPresets::PresetDocument> findFactoryPresetByName(
        const std::vector<PluginPresets::PresetDocument>& catalog,
        const juce::String& name) const;
    [[nodiscard]] const PluginPresets::PresetDocument* getSelectedDocument(
        const std::vector<PluginPresets::PresetDocument>& catalog) const;
    void resolveSelectionFromCurrentState();
    void markSnapshotDirty() noexcept;
    [[nodiscard]] PluginPresets::PresetUiSnapshot buildSnapshot(
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
    mutable std::optional<PluginPresets::PresetUiSnapshot> snapshotCache;
    mutable std::atomic<bool> snapshotDirty { true };
    std::optional<PluginPresets::PresetId> selectedPresetId;
};
