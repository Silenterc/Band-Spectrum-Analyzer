#pragma once

#include <vector>

#include "PresetDocumentSerializer.h"

class FactoryPresetRepository final {
public:
    explicit FactoryPresetRepository(const PluginPresets::PluginStateSnapshot& defaultState);
    explicit FactoryPresetRepository(std::vector<PluginPresets::PresetDocument> presetDocumentsToUse);

    [[nodiscard]] std::vector<PluginPresets::PresetDocument> loadAll() const;

private:
    [[nodiscard]] std::vector<PluginPresets::PresetDocument> loadEmbeddedPresetDocuments(
        const PluginPresets::PluginStateSnapshot& defaultState) const;

    std::vector<PluginPresets::PresetDocument> presetDocuments;
    PresetDocumentSerializer documentSerializer;
};
