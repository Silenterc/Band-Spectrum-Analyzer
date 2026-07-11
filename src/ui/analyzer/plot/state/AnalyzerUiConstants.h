#pragma once

#include <array>

#include "dsp/core/AnalyzerConstants.h"

namespace Ui::AnalyzerConstants {
    inline constexpr float maxUiFrequencyHz = 20000.0f;
    inline constexpr float defaultVisibleMinFrequencyHz = Analyzer::Constants::minFrequencyHz;
    inline constexpr float defaultVisibleMaxFrequencyHz = maxUiFrequencyHz;

    inline constexpr std::array<float, 10> frequencyScaleLabelsHz{
        20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, maxUiFrequencyHz
    };
}
