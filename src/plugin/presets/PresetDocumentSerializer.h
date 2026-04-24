#pragma once

#include <optional>

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#include "PresetTypes.h"

class PresetDocumentSerializer final {
public:
    static constexpr int currentFormatVersion = 1;

    [[nodiscard]] std::unique_ptr<juce::XmlElement> toXml(const PluginPresets::PresetDocument& document) const;
    [[nodiscard]] std::optional<PluginPresets::PresetDocument> fromXml(const juce::XmlElement& xml) const;
};
