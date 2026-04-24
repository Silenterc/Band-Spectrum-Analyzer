#include "PresetSession.h"

namespace {
    int comparePresetNames(const PluginPresets::PresetDocument& lhs, const PluginPresets::PresetDocument& rhs) {
        return lhs.name.compareIgnoreCase(rhs.name);
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

const PluginPresets::PresetUiSnapshot& PresetSession::getSnapshot() const {
    if (!snapshotCache.has_value() || snapshotDirty.exchange(false, std::memory_order_acq_rel))
        snapshotCache = buildSnapshot(getMergedCatalog(), captureCurrentState());

    return *snapshotCache;
}

std::optional<PluginPresets::PresetId> PresetSession::getSelectedPresetId() const noexcept {
    return selectedPresetId;
}

PluginPresets::PresetActionResult PresetSession::loadPreset(const PluginPresets::PresetId& presetId) {
    const auto& catalog = getMergedCatalog();
    const auto document = findPresetById(catalog, presetId);
    if (!document.has_value() || !document->pluginState.isValid()) {
        return PluginPresets::PresetActionResult::failed(
            PluginPresets::PresetActionErrorCode::presetNotLoadable,
            document.has_value() ? document->name : juce::String{});
    }

    if (!stateSerializer.applyState(document->pluginState, parameters, signalSlotOrderState)) {
        return PluginPresets::PresetActionResult::failed(
            PluginPresets::PresetActionErrorCode::presetNotLoadable,
            document->name);
    }

    currentStateSnapshotCache = document->pluginState;
    currentStateSnapshotDirty.store(false, std::memory_order_release);
    selectedPresetId = document->id;
    markSnapshotDirty();
    return PluginPresets::PresetActionResult::ok();
}

PluginPresets::PresetActionResult PresetSession::loadPreviousPreset() {
    if (!selectedPresetId.has_value())
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

    const auto& catalog = getMergedCatalog();
    const auto currentIndex = findPresetIndex(catalog, *selectedPresetId);
    if (!currentIndex.has_value() || *currentIndex == 0)
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

    return loadPreset(catalog[*currentIndex - 1].id);
}

PluginPresets::PresetActionResult PresetSession::loadNextPreset() {
    if (!selectedPresetId.has_value())
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

    const auto& catalog = getMergedCatalog();
    const auto currentIndex = findPresetIndex(catalog, *selectedPresetId);
    if (!currentIndex.has_value() || *currentIndex + 1 >= catalog.size())
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

    return loadPreset(catalog[*currentIndex + 1].id);
}

PluginPresets::PresetActionResult PresetSession::resetCurrentPreset() {
    if (selectedPresetId.has_value()) {
        const auto result = loadPreset(*selectedPresetId);
        if (result.succeeded)
            return result;
    }

    const auto& catalog = getMergedCatalog();
    if (catalog.empty())
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

    return loadPreset(catalog.front().id);
}

PluginPresets::PresetActionResult PresetSession::savePresetAs(const juce::String& name) {
    const auto trimmedName = name.trim();
    if (trimmedName.isEmpty())
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::emptyName);

    const auto& catalog = getMergedCatalog();
    if (findExistingUserPresetByName(catalog, trimmedName).has_value()
        || findFactoryPresetByName(factoryCatalog, trimmedName).has_value()) {
        return PluginPresets::PresetActionResult::failed(
            PluginPresets::PresetActionErrorCode::duplicateUserPresetName,
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
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::saveFailed,
                                                         document.name);
    }

    currentStateSnapshotCache = document.pluginState;
    currentStateSnapshotDirty.store(false, std::memory_order_release);
    invalidateMergedCatalog();
    selectedPresetId = document.id;
    markSnapshotDirty();
    return PluginPresets::PresetActionResult::ok();
}

PluginPresets::PresetActionResult PresetSession::overwritePreset(const PluginPresets::PresetId& presetId,
                                                                 const juce::String& name) {
    const auto trimmedName = name.trim();
    if (trimmedName.isEmpty())
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::emptyName);

    const auto& catalog = getMergedCatalog();
    const auto matchingFactoryDocument = findFactoryPresetById(factoryCatalog, presetId);

    if (matchingFactoryDocument.has_value()) {
        if (!matchingFactoryDocument->name.equalsIgnoreCase(trimmedName)) {
            return PluginPresets::PresetActionResult::failed(
                PluginPresets::PresetActionErrorCode::invalidOverwriteTarget,
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
            return PluginPresets::PresetActionResult::failed(
                PluginPresets::PresetActionErrorCode::overwriteFailed,
                matchingFactoryDocument->name);
        }

        currentStateSnapshotCache = overrideDocument.pluginState;
        currentStateSnapshotDirty.store(false, std::memory_order_release);
        invalidateMergedCatalog();
        selectedPresetId = overrideDocument.id;
        markSnapshotDirty();
        return PluginPresets::PresetActionResult::ok();
    }

    const auto duplicateNameDocument = findExistingUserPresetByName(catalog, trimmedName);
    if (duplicateNameDocument.has_value() && duplicateNameDocument->id != presetId) {
        return PluginPresets::PresetActionResult::failed(
            PluginPresets::PresetActionErrorCode::duplicateUserPresetName,
            trimmedName);
    }

    const auto existingDocument = userPresetStore.load(presetId);
    if (!existingDocument.has_value())
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::invalidOverwriteTarget);

    auto updatedDocument = *existingDocument;
    updatedDocument.name = trimmedName;
    updatedDocument.updatedAtUtc = juce::Time::getCurrentTime().toISO8601(true);
    if (updatedDocument.createdAtUtc.isEmpty())
        updatedDocument.createdAtUtc = updatedDocument.updatedAtUtc;
    updatedDocument.pluginState = captureCurrentState();

    if (!userPresetStore.save(updatedDocument)) {
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::overwriteFailed,
                                                         updatedDocument.name);
    }

    currentStateSnapshotCache = updatedDocument.pluginState;
    currentStateSnapshotDirty.store(false, std::memory_order_release);
    invalidateMergedCatalog();
    selectedPresetId = updatedDocument.id;
    markSnapshotDirty();
    return PluginPresets::PresetActionResult::ok();
}

PluginPresets::PresetActionResult PresetSession::deletePreset(const PluginPresets::PresetId& presetId) {
    const auto existingDocument = userPresetStore.load(presetId);
    if (!existingDocument.has_value())
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

    if (!userPresetStore.remove(presetId)) {
        return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::deleteFailed,
                                                         existingDocument->name);
    }

    invalidateMergedCatalog();
    if (selectedPresetId == std::optional<PluginPresets::PresetId>(presetId))
        selectedPresetId.reset();

    if (!selectedPresetId.has_value())
        resolveSelectionFromCurrentState();

    markSnapshotDirty();
    return PluginPresets::PresetActionResult::ok();
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

void PresetSession::restoreSelection(const std::optional<PluginPresets::PresetId>& presetId) {
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
    const PluginPresets::PresetId& presetId) const {
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
    const PluginPresets::PresetId& presetId) const {
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
    const PluginPresets::PresetId& presetId) const {
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
    selectedPresetId = iterator != catalog.end() ? std::optional<PluginPresets::PresetId>(iterator->id) : std::nullopt;
}

void PresetSession::markSnapshotDirty() noexcept {
    snapshotDirty.store(true, std::memory_order_release);
}

PluginPresets::PresetUiSnapshot PresetSession::buildSnapshot(
    const std::vector<PluginPresets::PresetDocument>& catalog,
    const PluginPresets::PluginStateSnapshot& currentState) const {
    PluginPresets::PresetUiSnapshot snapshot;
    snapshot.presets.reserve(catalog.size());

    for (const auto& document : catalog) {
        snapshot.presets.push_back({
            document.id,
            document.name,
            document.origin,
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
                                   ? PluginPresets::PresetSelectionStatus::unsaved
                                   : (stateSerializer.statesEqual(currentState, selectedDocument->pluginState)
                                          ? PluginPresets::PresetSelectionStatus::selectedClean
                                          : PluginPresets::PresetSelectionStatus::selectedDirty);
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
