#include "UserPresetStore.h"

namespace {
#if defined(JucePlugin_Name)
    constexpr auto defaultPresetProductName = JucePlugin_Name;
#else
    constexpr auto defaultPresetProductName = "band-spectrum-analyzer";
#endif

    juce::String sanitiseFileStem(const juce::String& name) {
        const auto legalName = juce::File::createLegalFileName(name.trim());
        return legalName.isNotEmpty() ? legalName : juce::String("Preset");
    }
}

UserPresetStore::UserPresetStore(juce::File directoryToUse)
    : directory(std::move(directoryToUse)) {
}

juce::File UserPresetStore::defaultPresetDirectory() {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(defaultPresetProductName)
        .getChildFile("Presets");
}

std::vector<PluginPresets::PresetDocument> UserPresetStore::loadAll() const {
    std::vector<PluginPresets::PresetDocument> documents;
    const auto storedDocuments = loadStoredDocuments();
    documents.reserve(storedDocuments.size());

    for (const auto& storedDocument : storedDocuments)
        documents.push_back(storedDocument.document);

    return documents;
}

std::optional<PluginPresets::PresetDocument> UserPresetStore::load(const PluginPresets::PresetId& presetId) const {
    const auto storedDocuments = loadStoredDocuments();
    const auto iterator = std::find_if(storedDocuments.begin(), storedDocuments.end(),
                                       [&presetId](const StoredDocument& storedDocument) {
                                           return storedDocument.document.id == presetId;
                                       });
    if (iterator == storedDocuments.end())
        return std::nullopt;

    return iterator->document;
}

bool UserPresetStore::save(const PluginPresets::PresetDocument& document) const {
    if (document.id.isEmpty() || document.name.trim().isEmpty() || !document.pluginState.isValid())
        return false;

    if (!directory.exists() && !directory.createDirectory())
        return false;

    const auto storedDocuments = loadStoredDocuments();
    const auto existingIterator = std::find_if(storedDocuments.begin(), storedDocuments.end(),
                                               [&document](const StoredDocument& storedDocument) {
                                                   return storedDocument.document.id == document.id;
                                               });

    const auto targetFile = directory.getChildFile(buildFileName(document));
    juce::TemporaryFile temporaryFile(targetFile);
    {
        auto outputStream = temporaryFile.getFile().createOutputStream();
        if (outputStream == nullptr)
            return false;

        const auto xml = documentSerializer.toXml(document);
        if (xml == nullptr)
            return false;

        xml->writeTo(*outputStream, {});
        outputStream->flush();
    }

    if (!temporaryFile.overwriteTargetFileWithTemporary())
        return false;

    if (existingIterator != storedDocuments.end() && existingIterator->file != targetFile)
        existingIterator->file.deleteFile();

    return true;
}

bool UserPresetStore::remove(const PluginPresets::PresetId& presetId) const {
    const auto storedDocuments = loadStoredDocuments();
    const auto iterator = std::find_if(storedDocuments.begin(), storedDocuments.end(),
                                       [&presetId](const StoredDocument& storedDocument) {
                                           return storedDocument.document.id == presetId;
                                       });
    if (iterator == storedDocuments.end())
        return false;

    return iterator->file.deleteFile();
}

std::vector<UserPresetStore::StoredDocument> UserPresetStore::loadStoredDocuments() const {
    std::vector<StoredDocument> storedDocuments;
    if (!directory.exists())
        return storedDocuments;

    const auto files = directory.findChildFiles(juce::File::findFiles, false, "*.xml");
    storedDocuments.reserve(static_cast<size_t>(files.size()));

    for (const auto& file : files) {
        std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(file));
        if (xml == nullptr)
            continue;

        const auto document = documentSerializer.fromXml(*xml);
        if (!document.has_value() || document->origin != PluginPresets::PresetOrigin::user)
            continue;

        storedDocuments.push_back({*document, file});
    }

    return storedDocuments;
}

juce::String UserPresetStore::buildFileName(const PluginPresets::PresetDocument& document) const {
    return sanitiseFileStem(document.name) + "__" + juce::File::createLegalFileName(document.id) + ".xml";
}
