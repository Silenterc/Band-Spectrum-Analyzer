#include "PresetDocumentSerializer.h"

namespace {
    constexpr auto presetDocumentType = "PresetDocument";
    constexpr auto formatVersionProperty = "formatVersion";
    constexpr auto idProperty = "id";
    constexpr auto nameProperty = "name";
    constexpr auto originProperty = "origin";
    constexpr auto createdAtProperty = "createdAtUtc";
    constexpr auto updatedAtProperty = "updatedAtUtc";
}

std::unique_ptr<juce::XmlElement> PresetDocumentSerializer::toXml(const PluginPresets::PresetDocument& document) const {
    juce::ValueTree tree(presetDocumentType);
    tree.setProperty(formatVersionProperty, document.formatVersion, nullptr);
    tree.setProperty(idProperty, document.id, nullptr);
    tree.setProperty(nameProperty, document.name, nullptr);
    tree.setProperty(originProperty, PluginPresets::toString(document.origin), nullptr);
    tree.setProperty(createdAtProperty, document.createdAtUtc, nullptr);
    tree.setProperty(updatedAtProperty, document.updatedAtUtc, nullptr);

    if (document.pluginState.isValid())
        tree.addChild(document.pluginState.state.createCopy(), -1, nullptr);

    return tree.createXml();
}

std::optional<PluginPresets::PresetDocument> PresetDocumentSerializer::fromXml(const juce::XmlElement& xml) const {
    const auto tree = juce::ValueTree::fromXml(xml);
    if (!tree.isValid() || tree.getType() != juce::Identifier(presetDocumentType))
        return std::nullopt;

    const auto origin = PluginPresets::presetOriginFromString(tree.getProperty(originProperty).toString());
    if (!origin.has_value())
        return std::nullopt;

    PluginPresets::PresetDocument document;
    document.formatVersion = static_cast<int>(tree.getProperty(formatVersionProperty, currentFormatVersion));
    document.id = tree.getProperty(idProperty).toString();
    document.name = tree.getProperty(nameProperty).toString();
    document.origin = *origin;
    document.createdAtUtc = tree.getProperty(createdAtProperty).toString();
    document.updatedAtUtc = tree.getProperty(updatedAtProperty).toString();

    if (document.id.isEmpty() || document.name.isEmpty())
        return std::nullopt;

    if (tree.getNumChildren() > 0)
        document.pluginState.state = tree.getChild(0).createCopy();

    return document;
}
