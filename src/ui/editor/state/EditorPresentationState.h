#pragma once

#include "shared/UiScalePreset.h"

namespace Ui {
    struct EditorPresentationState {
        UiScalePreset scale = UiScalePreset::x1;
    };

    inline bool operator==(const EditorPresentationState &lhs, const EditorPresentationState &rhs) {
        return lhs.scale == rhs.scale;
    }

    inline bool operator!=(const EditorPresentationState &lhs, const EditorPresentationState &rhs) {
        return !(lhs == rhs);
    }
}
