#include "PresetTypes.h"

namespace PluginPresets {
    bool PluginStateSnapshot::isValid() const {
        return state.isValid();
    }

    juce::String toString(const PresetOrigin origin) {
        switch (origin) {
            case PresetOrigin::factory:
                return "factory";
            case PresetOrigin::user:
                return "user";
        }

        jassertfalse;
        return "factory";
    }

    std::optional<PresetOrigin> presetOriginFromString(const juce::String& text) {
        if (text.equalsIgnoreCase("factory"))
            return PresetOrigin::factory;

        if (text.equalsIgnoreCase("user"))
            return PresetOrigin::user;

        return std::nullopt;
    }

    bool operator==(const PluginStateSnapshot& lhs, const PluginStateSnapshot& rhs) {
        if (!lhs.state.isValid() || !rhs.state.isValid())
            return lhs.state.isValid() == rhs.state.isValid();

        return lhs.state.isEquivalentTo(rhs.state);
    }

}
