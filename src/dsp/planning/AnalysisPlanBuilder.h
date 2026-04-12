#pragma once

#include <optional>
#include <vector>

#include "../core/AnalyzerData.h"
#include "../core/EngineParameterState.h"

namespace Analyzer {
    /**
     * Which input family an analysis group should read from
     */
    enum class SourceFamily {
        mainInput,
        sidechain
    };

    /**
     * Which derived signal inside a source family one lane should consume
     */
    enum class DerivedSignal {
        mid,
        left,
        right,
        side
    };

    struct AnalysisGroupSpec {
        TraceKind kind = TraceKind::slot1;
        SourceFamily sourceFamily = SourceFamily::mainInput;
        DerivedSignal primarySignal = DerivedSignal::mid;
        // Used for stereo (left + right)
        std::optional<DerivedSignal> secondarySignal;
    };

    /**
     * Builds the active analysis plan for the current parameter state.
     */
    class AnalysisPlanBuilder {
    public:
        std::vector<AnalysisGroupSpec> build(const EngineParameterState &parameters) const;
    };
}
