#pragma once

#include <optional>
#include <vector>

#include <juce_core/juce_core.h>

#include "PresetDocumentSerializer.h"

class UserPresetStore final {
public:
    explicit UserPresetStore(juce::File directoryToUse);

    [[nodiscard]] static juce::File defaultPresetDirectory();

    [[nodiscard]] std::vector<PluginPresets::PresetDocument> loadAll() const;
    [[nodiscard]] std::optional<PluginPresets::PresetDocument> load(const PluginPresets::PresetId& presetId) const;
    bool save(const PluginPresets::PresetDocument& document) const;
    bool remove(const PluginPresets::PresetId& presetId) const;

private:
    struct StoredDocument {
        PluginPresets::PresetDocument document;
        juce::File file;
    };

    [[nodiscard]] std::vector<StoredDocument> loadStoredDocuments() const;
    [[nodiscard]] juce::String buildFileName(const PluginPresets::PresetDocument& document) const;

    juce::File directory;
    PresetDocumentSerializer documentSerializer;
};
