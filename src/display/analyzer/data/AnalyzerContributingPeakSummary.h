#pragma once

#include <optional>
#include <vector>

#include "dsp/core/AnalyzerData.h"

struct AnalyzerContributingPeakSummary {
    std::vector<float> peakDb;
    std::vector<std::optional<Analyzer::TraceKind>> ownerKinds;
};
