#pragma once

#include "plugin/presets/PresetTypes.h"

class PresetUiSnapshotSource {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void presetUiSnapshotChanged(const PluginPresets::PresetUiSnapshot& snapshot) = 0;
    };

    virtual ~PresetUiSnapshotSource() = default;

    virtual PluginPresets::PresetUiSnapshot getPresetUiSnapshot() const = 0;
    virtual void addPresetUiSnapshotListener(Listener& listener) = 0;
    virtual void removePresetUiSnapshotListener(Listener& listener) = 0;
};
