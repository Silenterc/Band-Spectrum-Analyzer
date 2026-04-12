#pragma once

#include <optional>
#include <vector>

#include "dsp/core/AnalyzerData.h"

struct AnalyzerGlobalHoldFrame {
    std::vector<float> holdDb;
    std::vector<std::optional<Analyzer::TraceKind>> ownerKinds;
};
