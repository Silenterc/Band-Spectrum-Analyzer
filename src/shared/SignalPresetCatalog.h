#pragma once

#include <array>

#include <juce_core/juce_core.h>

namespace Shared {
    struct SignalPresetSpec {
        const char *name = "";
        juce::uint32 argb = 0xff000000;
    };

    inline constexpr std::array<SignalPresetSpec, 10> signalPresetCatalog{{
        {"Cyan", 0xff58d7ff},
        {"Lime", 0xff92df51},
        {"Amber", 0xffffbf47},
        {"Coral", 0xffff7f66},
        {"Violet", 0xff8e79ff},
        {"Magenta", 0xffff6fd7},
        {"Teal", 0xff3ed0c4},
        {"Red", 0xffff5c67},
        {"Mint", 0xff8ef0c2},
        {"Gold", 0xffffd166}
    }};

    inline constexpr int signalPresetCount = static_cast<int>(signalPresetCatalog.size());
}
