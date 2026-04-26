#include "PresetSession.h"

namespace {
    int comparePresetNames(const PluginPresets::PresetDocument& lhs, const PluginPresets::PresetDocument& rhs) {
        return lhs.name.compareIgnoreCase(rhs.name);
    }

    Ui::Presets::PresetOrigin toUiPresetOrigin(const PluginPresets::PresetOrigin origin) {
        switch (origin) {
            case PluginPresets::PresetOrigin::factory:
                return Ui::Presets::PresetOrigin::factory;
            case PluginPresets::PresetOrigin::user:
                return Ui::Presets::PresetOrigin::user;
        }

        jassertfalse;
        return Ui::Presets::PresetOrigin::factory;
    }
}

PresetSession::PresetSession(juce::AudioProcessorValueTreeState& parametersToUse,
                             SignalSlotOrderState& signalSlotOrderStateToUse,
                             PluginStateSerializer& stateSerializerToUse,
                             FactoryPresetRepository& factoryRepositoryToUse,
                             UserPresetStore& userPresetStoreToUse)
    : parameters(parametersToUse),
      signalSlotOrderState(signalSlotOrderStateToUse),
      stateSerializer(stateSerializerToUse),
      userPresetStore(userPresetStoreToUse),
      factoryCatalog(factoryRepositoryToUse.loadAll()) {
    resolveSelectionFromCurrentState();
}

const Ui::Presets::PresetUiSnapshot& PresetSession::getSnapshot() const {
    if (!snapshotCache.has_value() || snapshotDirty.exchange(false, std::memory_order_acq_rel))
        snapshotCache = buildSnapshot(getMergedCatalog(), captureCurrentState());

    return *snapshotCache;
}

std::optional<Ui::Presets::PresetId> PresetSession::getSelectedPresetId() const noexcept {
    return selectedPresetId;
}

Ui::Presets::PresetActionResult PresetSession::loadPreset(const Ui::Presets::PresetId& presetId) {
    const auto& catalog = getMergedCatalog();
    const auto document = findPresetById(catalog, presetId);
    if (!document.has_value() || !document->pluginState.isValid()) {
        return Ui::Presets::PresetActionResult::failed(
            Ui::Presets::PresetActionErrorCode::presetNotLoadable,
            document.has_value() ? document->name : juce::String{});
    }

    if (!stateSerializer.applyState(document->pluginState, parameters, signalSlotOrderState)) {
        return Ui::Presets::PresetActionResult::failed(
            Ui::Presets::PresetActionErrorCode::presetNotLoadable,
            document->name);
    }

    currentStateSnapshotCache = document->pluginState;
    currentStateSnapshotDirty.store(false, std::memory_order_release);
    selectedPresetId = document->id;
    markSnapshotDirty();
    return Ui::Presets::PresetActionResult::ok();
}

Ui::Presets::PresetActionResult PresetSession::loadPreviousPreset() {
    if (!selectedPresetId.has_value())
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::presetNotFound);

    const auto& catalog = getMergedCatalog();
    const auto currentIndex = findPresetIndex(catalog, *selectedPresetId);
    if (!currentIndex.has_value() || *currentIndex == 0)
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::presetNotFound);

    return loadPreset(catalog[*currentIndex - 1].id);
}

Ui::Presets::PresetActionResult PresetSession::loadNextPreset() {
    if (!selectedPresetId.has_value())
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::presetNotFound);

    const auto& catalog = getMergedCatalog();
    const auto currentIndex = findPresetIndex(catalog, *selectedPresetId);
    if (!currentIndex.has_value() || *currentIndex + 1 >= catalog.size())
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::presetNotFound);

    return loadPreset(catalog[*currentIndex + 1].id);
}

Ui::Presets::PresetActionResult PresetSession::resetCurrentPreset() {
    if (selectedPresetId.has_value()) {
        const auto result = loadPreset(*selectedPresetId);
        if (result.succeeded)
            return result;
    }

    const auto& catalog = getMergedCatalog();
    if (catalog.empty())
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::presetNotFound);

    return loadPreset(catalog.front().id);
}

Ui::Presets::PresetActionResult PresetSession::savePresetAs(const juce::String& name) {
    const auto trimmedName = name.trim();
    if (trimmedName.isEmpty())
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::emptyName);

    const auto& catalog = getMergedCatalog();
    if (findExistingUserPresetByName(catalog, trimmedName).has_value()
        || findFactoryPresetByName(factoryCatalog, trimmedName).has_value()) {
        return Ui::Presets::PresetActionResult::failed(
            Ui::Presets::PresetActionErrorCode::duplicateUserPresetName,
            trimmedName);
    }

    PluginPresets::PresetDocument document;
    document.id = juce::Uuid().toString();
    document.name = trimmedName;
    document.origin = PluginPresets::PresetOrigin::user;
    document.createdAtUtc = juce::Time::getCurrentTime().toISO8601(true);
    document.updatedAtUtc = document.createdAtUtc;
    document.pluginState = captureCurrentState();

    if (!userPresetStore.save(document)) {
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::saveFailed,
                                                         document.name);
    }

    currentStateSnapshotCache = document.pluginState;
    currentStateSnapshotDirty.store(false, std::memory_order_release);
    invalidateMergedCatalog();
    selectedPresetId = document.id;
    markSnapshotDirty();
    return Ui::Presets::PresetActionResult::ok();
}

Ui::Presets::PresetActionResult PresetSession::overwritePreset(const Ui::Presets::PresetId& presetId,
                                                                 const juce::String& name) {
    const auto trimmedName = name.trim();
    if (trimmedName.isEmpty())
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::emptyName);

    const auto& catalog = getMergedCatalog();
    const auto matchingFactoryDocument = findFactoryPresetById(factoryCatalog, presetId);

    if (matchingFactoryDocument.has_value()) {
        if (!matchingFactoryDocument->name.equalsIgnoreCase(trimmedName)) {
            return Ui::Presets::PresetActionResult::failed(
                Ui::Presets::PresetActionErrorCode::invalidOverwriteTarget,
                matchingFactoryDocument->name);
        }

        PluginPresets::PresetDocument overrideDocument;
        if (const auto existingOverride = userPresetStore.load(presetId); existingOverride.has_value())
            overrideDocument = *existingOverride;

        overrideDocument.id = matchingFactoryDocument->id;
        overrideDocument.name = matchingFactoryDocument->name;
        overrideDocument.origin = PluginPresets::PresetOrigin::user;
        overrideDocument.updatedAtUtc = juce::Time::getCurrentTime().toISO8601(true);
        if (overrideDocument.createdAtUtc.isEmpty())
            overrideDocument.createdAtUtc = overrideDocument.updatedAtUtc;
        overrideDocument.pluginState = captureCurrentState();

        if (!userPresetStore.save(overrideDocument)) {
            return Ui::Presets::PresetActionResult::failed(
                Ui::Presets::PresetActionErrorCode::overwriteFailed,
                matchingFactoryDocument->name);
        }

        currentStateSnapshotCache = overrideDocument.pluginState;
        currentStateSnapshotDirty.store(false, std::memory_order_release);
        invalidateMergedCatalog();
        selectedPresetId = overrideDocument.id;
        markSnapshotDirty();
        return Ui::Presets::PresetActionResult::ok();
    }

    const auto duplicateNameDocument = findExistingUserPresetByName(catalog, trimmedName);
    if (duplicateNameDocument.has_value() && duplicateNameDocument->id != presetId) {
        return Ui::Presets::PresetActionResult::failed(
            Ui::Presets::PresetActionErrorCode::duplicateUserPresetName,
            trimmedName);
    }

    const auto existingDocument = userPresetStore.load(presetId);
    if (!existingDocument.has_value())
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::invalidOverwriteTarget);

    auto updatedDocument = *existingDocument;
    updatedDocument.name = trimmedName;
    updatedDocument.updatedAtUtc = juce::Time::getCurrentTime().toISO8601(true);
    if (updatedDocument.createdAtUtc.isEmpty())
        updatedDocument.createdAtUtc = updatedDocument.updatedAtUtc;
    updatedDocument.pluginState = captureCurrentState();

    if (!userPresetStore.save(updatedDocument)) {
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::overwriteFailed,
                                                         updatedDocument.name);
    }

    currentStateSnapshotCache = updatedDocument.pluginState;
    currentStateSnapshotDirty.store(false, std::memory_order_release);
    invalidateMergedCatalog();
    selectedPresetId = updatedDocument.id;
    markSnapshotDirty();
    return Ui::Presets::PresetActionResult::ok();
}

Ui::Presets::PresetActionResult PresetSession::deletePreset(const Ui::Presets::PresetId& presetId) {
    const auto existingDocument = userPresetStore.load(presetId);
    if (!existingDocument.has_value())
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::presetNotFound);

    if (!userPresetStore.remove(presetId)) {
        return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::deleteFailed,
                                                         existingDocument->name);
    }

    invalidateMergedCatalog();
    if (selectedPresetId == std::optional<Ui::Presets::PresetId>(presetId))
        selectedPresetId.reset();

    if (!selectedPresetId.has_value())
        resolveSelectionFromCurrentState();

    markSnapshotDirty();
    return Ui::Presets::PresetActionResult::ok();
}

void PresetSession::refreshCatalog() {
    invalidateMergedCatalog();

    const auto& catalog = getMergedCatalog();
    if (selectedPresetId.has_value() && !findPresetById(catalog, *selectedPresetId).has_value())
        selectedPresetId.reset();

    if (!selectedPresetId.has_value())
        resolveSelectionFromCurrentState();

    markSnapshotDirty();
}

void PresetSession::restoreSelection(const std::optional<Ui::Presets::PresetId>& presetId) {
    if (presetId.has_value()) {
        const auto& catalog = getMergedCatalog();
        const auto selectedDocument = findPresetById(catalog, *presetId);
        if (selectedDocument.has_value()) {
            selectedPresetId = selectedDocument->id;
            markSnapshotDirty();
            return;
        }
    }

    selectedPresetId.reset();
    resolveSelectionFromCurrentState();
    markSnapshotDirty();
}

void PresetSession::markCurrentStateDirty() noexcept {
    currentStateSnapshotDirty.store(true, std::memory_order_release);
    markSnapshotDirty();
}

const PluginPresets::PluginStateSnapshot& PresetSession::captureCurrentState() const {
    if (!currentStateSnapshotCache.has_value()
        || currentStateSnapshotDirty.exchange(false, std::memory_order_acq_rel)) {
        currentStateSnapshotCache = stateSerializer.captureState(parameters, signalSlotOrderState);
    }

    return *currentStateSnapshotCache;
}

const std::vector<PluginPresets::PresetDocument>& PresetSession::getMergedCatalog() const {
    if (mergedCatalogDirty)
        rebuildMergedCatalog();

    return mergedCatalogCache;
}

void PresetSession::invalidateMergedCatalog() {
    mergedCatalogDirty = true;
    markSnapshotDirty();
}

void PresetSession::rebuildMergedCatalog() const {
    auto catalog = factoryCatalog;
    auto userPresets = userPresetStore.loadAll();
    std::sort(userPresets.begin(), userPresets.end(), [](const auto& lhs, const auto& rhs) {
        return comparePresetNames(lhs, rhs) < 0;
    });

    for (const auto& userPreset : userPresets) {
        const auto factoryIterator = std::find_if(catalog.begin(), catalog.end(),
                                                  [&userPreset](const PluginPresets::PresetDocument& document) {
                                                      return document.id == userPreset.id;
                                                  });
        if (factoryIterator != catalog.end()) {
            auto shadowDocument = userPreset;
            shadowDocument.name = factoryIterator->name;
            shadowDocument.origin = PluginPresets::PresetOrigin::user;
            *factoryIterator = std::move(shadowDocument);
            continue;
        }

        catalog.push_back(userPreset);
    }

    mergedCatalogCache = std::move(catalog);
    mergedCatalogDirty = false;
}

std::optional<PluginPresets::PresetDocument> PresetSession::findPresetById(
    const std::vector<PluginPresets::PresetDocument>& catalog,
    const Ui::Presets::PresetId& presetId) const {
    const auto iterator = std::find_if(catalog.begin(), catalog.end(),
                                       [&presetId](const PluginPresets::PresetDocument& document) {
                                           return document.id == presetId;
                                       });
    if (iterator == catalog.end())
        return std::nullopt;

    return *iterator;
}

std::optional<size_t> PresetSession::findPresetIndex(
    const std::vector<PluginPresets::PresetDocument>& catalog,
    const Ui::Presets::PresetId& presetId) const {
    const auto iterator = std::find_if(catalog.begin(), catalog.end(),
                                       [&presetId](const PluginPresets::PresetDocument& document) {
                                           return document.id == presetId;
                                       });
    if (iterator == catalog.end())
        return std::nullopt;

    return static_cast<size_t>(std::distance(catalog.begin(), iterator));
}

std::optional<PluginPresets::PresetDocument> PresetSession::findExistingUserPresetByName(
    const std::vector<PluginPresets::PresetDocument>& catalog,
    const juce::String& name) const {
    const auto trimmedName = name.trim();
    const auto iterator = std::find_if(catalog.begin(), catalog.end(),
                                       [&trimmedName](const PluginPresets::PresetDocument& document) {
                                           return document.origin == PluginPresets::PresetOrigin::user
                                                  && document.name.equalsIgnoreCase(trimmedName);
                                       });
    if (iterator == catalog.end())
        return std::nullopt;

    return *iterator;
}

std::optional<PluginPresets::PresetDocument> PresetSession::findFactoryPresetById(
    const std::vector<PluginPresets::PresetDocument>& catalog,
    const Ui::Presets::PresetId& presetId) const {
    const auto iterator = std::find_if(catalog.begin(), catalog.end(),
                                       [&presetId](const PluginPresets::PresetDocument& document) {
                                           return document.origin == PluginPresets::PresetOrigin::factory
                                                  && document.id == presetId;
                                       });
    if (iterator == catalog.end())
        return std::nullopt;

    return *iterator;
}

std::optional<PluginPresets::PresetDocument> PresetSession::findFactoryPresetByName(
    const std::vector<PluginPresets::PresetDocument>& catalog,
    const juce::String& name) const {
    const auto trimmedName = name.trim();
    const auto iterator = std::find_if(catalog.begin(), catalog.end(),
                                       [&trimmedName](const PluginPresets::PresetDocument& document) {
                                           return document.origin == PluginPresets::PresetOrigin::factory
                                                  && document.name.equalsIgnoreCase(trimmedName);
                                       });
    if (iterator == catalog.end())
        return std::nullopt;

    return *iterator;
}

const PluginPresets::PresetDocument* PresetSession::getSelectedDocument(
    const std::vector<PluginPresets::PresetDocument>& catalog) const {
    if (!selectedPresetId.has_value())
        return nullptr;

    const auto iterator = std::find_if(catalog.begin(), catalog.end(),
                                       [this](const PluginPresets::PresetDocument& document) {
                                           return document.id == *selectedPresetId;
                                       });
    return iterator != catalog.end() ? &*iterator : nullptr;
}

void PresetSession::resolveSelectionFromCurrentState() {
    const auto& currentState = captureCurrentState();
    const auto& catalog = getMergedCatalog();
    const auto iterator = std::find_if(catalog.begin(), catalog.end(),
                                       [this, &currentState](const PluginPresets::PresetDocument& document) {
                                           return stateSerializer.statesEqual(currentState, document.pluginState);
                                       });
    selectedPresetId = iterator != catalog.end() ? std::optional<Ui::Presets::PresetId>(iterator->id) : std::nullopt;
}

void PresetSession::markSnapshotDirty() noexcept {
    snapshotDirty.store(true, std::memory_order_release);
}

Ui::Presets::PresetUiSnapshot PresetSession::buildSnapshot(
    const std::vector<PluginPresets::PresetDocument>& catalog,
    const PluginPresets::PluginStateSnapshot& currentState) const {
    Ui::Presets::PresetUiSnapshot snapshot;
    snapshot.presets.reserve(catalog.size());

    for (const auto& document : catalog) {
        snapshot.presets.push_back({
            document.id,
            document.name,
            toUiPresetOrigin(document.origin),
            document.origin == PluginPresets::PresetOrigin::user,
            findFactoryPresetById(factoryCatalog, document.id).has_value()
                && document.origin == PluginPresets::PresetOrigin::user
        });
    }

    snapshot.selectedPresetId = selectedPresetId;
    const auto* selectedDocument = getSelectedDocument(catalog);
    snapshot.selectedPresetName = selectedDocument != nullptr ? selectedDocument->name : juce::String("Unsaved");
    snapshot.isSelectedPresetUser = selectedDocument != nullptr
                                    && selectedDocument->origin == PluginPresets::PresetOrigin::user;
    snapshot.selectionStatus = selectedDocument == nullptr
                                   ? Ui::Presets::PresetSelectionStatus::unsaved
                                   : (stateSerializer.statesEqual(currentState, selectedDocument->pluginState)
                                          ? Ui::Presets::PresetSelectionStatus::selectedClean
                                          : Ui::Presets::PresetSelectionStatus::selectedDirty);
    snapshot.canReset = !catalog.empty();
    snapshot.canSave = true;
    if (selectedPresetId.has_value()) {
        const auto currentIndex = findPresetIndex(catalog, *selectedPresetId);
        if (currentIndex.has_value()) {
            snapshot.canLoadPrevious = *currentIndex > 0;
            snapshot.canLoadNext = *currentIndex + 1 < catalog.size();
        }
    }

    return snapshot;
}
