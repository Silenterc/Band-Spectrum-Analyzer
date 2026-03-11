#pragma once

#include <array>

namespace Shared {
    inline constexpr size_t maxSignalSlots = 4;
    using SignalSlotOrder = std::array<size_t, maxSignalSlots>;
}

namespace Analyzer {
    enum class SignalSource {
        main = 0,
        sidechain
    };

    enum class SignalMode {
        mid = 0,
        side,
        stereo
    };

    struct SignalSlotConfiguration {
        bool enabled = false;
        SignalSource source = SignalSource::main;
        SignalMode mode = SignalMode::mid;
    };

    inline bool operator==(const SignalSlotConfiguration &lhs, const SignalSlotConfiguration &rhs) {
        return lhs.enabled == rhs.enabled
               && lhs.source == rhs.source
               && lhs.mode == rhs.mode;
    }

    inline bool operator!=(const SignalSlotConfiguration &lhs, const SignalSlotConfiguration &rhs) {
        return !(lhs == rhs);
    }
}
