#pragma once

#include <algorithm>

#include "shared/DefaultParameterValues.h"
#include "ui/analyzer/state/AnalyzerUiSnapshot.h"

/**
 * Derives the interactive clamp windows that keep paired min/max settings controls
 * from crossing, using the shared span constants the plugin enforces with.
 */
namespace Ui::SettingsRangeModel {
    struct AllowedRange {
        float minimum;
        float maximum;
    };

    inline AllowedRange gridMinDbAllowedRange(const AnalyzerUiSnapshot& snapshot) {
        const auto maximum = std::clamp(snapshot.gridMaxDb - Defaults::gridMinSpanDb,
                                        Defaults::gridMinDbMin,
                                        Defaults::gridMinDbMax);
        return {Defaults::gridMinDbMin, maximum};
    }

    inline AllowedRange gridMaxDbAllowedRange(const AnalyzerUiSnapshot& snapshot) {
        const auto minimum = std::clamp(snapshot.gridMinDb + Defaults::gridMinSpanDb,
                                        Defaults::gridMaxDbMin,
                                        Defaults::gridMaxDbMax);
        return {minimum, Defaults::gridMaxDbMax};
    }

    inline AllowedRange visibleMinFrequencyAllowedRange(const AnalyzerUiSnapshot& snapshot) {
        const auto maximum = std::clamp(snapshot.visibleMaxFrequencyHz / Defaults::visibleFrequencyMinSpanRatio,
                                        Defaults::visibleMinFrequencyHzMin,
                                        Defaults::visibleMinFrequencyHzMax);
        return {Defaults::visibleMinFrequencyHzMin, maximum};
    }

    inline AllowedRange visibleMaxFrequencyAllowedRange(const AnalyzerUiSnapshot& snapshot) {
        const auto minimum = std::clamp(snapshot.visibleMinFrequencyHz * Defaults::visibleFrequencyMinSpanRatio,
                                        Defaults::visibleMaxFrequencyHzMin,
                                        Defaults::visibleMaxFrequencyHzMax);
        return {minimum, Defaults::visibleMaxFrequencyHzMax};
    }
}
