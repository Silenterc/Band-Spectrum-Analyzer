#pragma once

#include <array>

#include <juce_core/juce_core.h>

namespace Shared {
    struct SignalPresetSpec {
        const char *name = "";
        juce::uint32 argb = 0xff000000;
    };

    inline constexpr std::array<SignalPresetSpec, 8> signalPresetCatalog{{
        {"Blue", 0xff3b97ff},
        {"Lime", 0xffb8d63a},
        {"Amber", 0xffffbf47},
        {"Coral", 0xffff7854},
        {"Violet", 0xff7d67ea},
        {"Magenta", 0xffff58bf},
        {"Teal", 0xff24ccb3},
        {"Red", 0xffff5348}
    }};

    inline constexpr int signalPresetCount = static_cast<int>(signalPresetCatalog.size());
}
