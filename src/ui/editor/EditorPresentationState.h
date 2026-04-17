#pragma once

namespace Ui {
    enum class UiScalePreset {
        x1,
        x1_5,
        x2
    };

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
