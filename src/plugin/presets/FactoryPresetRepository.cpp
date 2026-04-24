#include "FactoryPresetRepository.h"

#include <BinaryData.h>

namespace {
    struct EmbeddedPresetAsset {
        const char* data = nullptr;
        int size = 0;
    };

    const EmbeddedPresetAsset embeddedFactoryPresets[]{
        {BinaryData::factory_default_preset_xml, BinaryData::factory_default_preset_xmlSize}
    };
}

FactoryPresetRepository::FactoryPresetRepository(const PluginPresets::PluginStateSnapshot& defaultState)
    : presetDocuments(loadEmbeddedPresetDocuments(defaultState)) {
}

FactoryPresetRepository::FactoryPresetRepository(std::vector<PluginPresets::PresetDocument> presetDocumentsToUse)
    : presetDocuments(std::move(presetDocumentsToUse)) {
}

std::vector<PluginPresets::PresetDocument> FactoryPresetRepository::loadAll() const {
    return presetDocuments;
}

std::vector<PluginPresets::PresetDocument> FactoryPresetRepository::loadEmbeddedPresetDocuments(
    const PluginPresets::PluginStateSnapshot& defaultState) const {
    std::vector<PluginPresets::PresetDocument> documents;
    documents.reserve(std::size(embeddedFactoryPresets));

    for (const auto& embeddedPreset : embeddedFactoryPresets) {
        if (embeddedPreset.data == nullptr || embeddedPreset.size <= 0)
            continue;

        const auto xml = juce::parseXML(juce::String::fromUTF8(embeddedPreset.data, embeddedPreset.size));
        if (xml == nullptr)
            continue;

        const auto document = documentSerializer.fromXml(*xml);
        if (!document.has_value())
            continue;

        auto patchedDocument = *document;
        if (!patchedDocument.pluginState.isValid())
            patchedDocument.pluginState = defaultState;

        documents.push_back(std::move(patchedDocument));
    }

    return documents;
}
