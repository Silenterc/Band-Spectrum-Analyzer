#pragma once

#include <array>

#include "BandMode.h"
#include "../shared/DefaultParameterValues.h"
#include "../shared/SignalSlotConfiguration.h"

namespace Analyzer {
    struct EngineParameterState {
        BandMode bandMode = Defaults::bandMode;
        std::array<SignalSlotConfiguration, Shared::maxSignalSlots> signalSlots{};
    };

    inline bool operator==(const EngineParameterState &lhs, const EngineParameterState &rhs) {
        return lhs.bandMode == rhs.bandMode
               && lhs.signalSlots == rhs.signalSlots;
    }

    inline bool operator!=(const EngineParameterState &lhs, const EngineParameterState &rhs) {
        return !(lhs == rhs);
    }
}
