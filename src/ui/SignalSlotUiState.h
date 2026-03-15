#pragma once

#include <array>
#include <cmath>

#include "../shared/DefaultParameterValues.h"
#include "../shared/SignalSlotConfiguration.h"
#include "../shared/SignalPresetCatalog.h"

namespace Ui {
    inline constexpr int signalPresetCount = Shared::signalPresetCount;
    inline constexpr float defaultSignalOpacity = Defaults::signalOpacity;

    struct SignalSlotState {
        Analyzer::SignalSlotConfiguration configuration;
        bool visible = true;
        int colourIndex = 0;
        float opacity = defaultSignalOpacity;
    };

    inline bool operator==(const SignalSlotState &lhs, const SignalSlotState &rhs) {
        return lhs.configuration == rhs.configuration
               && lhs.visible == rhs.visible
               && lhs.colourIndex == rhs.colourIndex
               && std::abs(lhs.opacity - rhs.opacity) <= 0.0001f;
    }

    inline bool operator!=(const SignalSlotState &lhs, const SignalSlotState &rhs) {
        return !(lhs == rhs);
    }
}
