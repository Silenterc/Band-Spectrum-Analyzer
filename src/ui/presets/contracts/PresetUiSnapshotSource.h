#pragma once

#include "ui/presets/state/PresetUiState.h"

class PresetUiSnapshotSource {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void presetUiSnapshotChanged(const Ui::Presets::PresetUiSnapshot& snapshot) = 0;
    };

    virtual ~PresetUiSnapshotSource() = default;

    virtual Ui::Presets::PresetUiSnapshot getPresetUiSnapshot() const = 0;
    virtual void addPresetUiSnapshotListener(Listener& listener) = 0;
    virtual void removePresetUiSnapshotListener(Listener& listener) = 0;
};
