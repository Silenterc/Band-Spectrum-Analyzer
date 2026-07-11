#pragma once

#include "shared/UiScalePreset.h"

class EditorPresentationActions {
public:
    virtual ~EditorPresentationActions() = default;

    virtual void setUiScalePreset(Ui::UiScalePreset preset) = 0;
};
