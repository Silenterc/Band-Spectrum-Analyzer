#pragma once

#include <array>
#include <cstddef>

#include "../dsp/core/BandMode.h"
#include "UiScalePreset.h"

namespace Shared {
    /** A selectable parameter option: enum value plus its user-facing label, defined once for plugin and UI. */
    template<typename Enum>
    struct EnumChoice {
        Enum value;
        const char *label;
    };

    inline constexpr std::array<EnumChoice<Analyzer::BandMode>, 4> bandModeChoices{{
        {Analyzer::BandMode::octaveThird, "1/3 Oct"},
        {Analyzer::BandMode::octaveQuarter, "1/4 Oct"},
        {Analyzer::BandMode::octaveSixth, "1/6 Oct"},
        {Analyzer::BandMode::octaveTwelfth, "1/12 Oct"}
    }};

    inline constexpr std::array<EnumChoice<Ui::UiScalePreset>, 3> uiScaleChoices{{
        {Ui::UiScalePreset::x1, "1x"},
        {Ui::UiScalePreset::x1_5, "1.5x"},
        {Ui::UiScalePreset::x2, "2x"}
    }};

    template<typename Enum, std::size_t Size>
    constexpr int indexForValue(const Enum value, const std::array<EnumChoice<Enum>, Size> &choices) {
        for (std::size_t index = 0; index < choices.size(); ++index) {
            if (choices[index].value == value)
                return static_cast<int>(index);
        }

        return 0;
    }

    template<typename Enum, std::size_t Size>
    constexpr Enum valueForIndex(const int index, const std::array<EnumChoice<Enum>, Size> &choices) {
        const auto highestIndex = static_cast<int>(choices.size()) - 1;
        const auto clampedIndex = index < 0 ? 0 : (index > highestIndex ? highestIndex : index);
        return choices[static_cast<std::size_t>(clampedIndex)].value;
    }
}
