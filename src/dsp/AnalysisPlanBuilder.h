#pragma once

#include <initializer_list>
#include <vector>

#include "AnalyzerData.h"
#include "EngineParameterState.h"

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

    /**
     * How one output trace should be formed from one or more processed lanes
     */
    enum class OutputMixMode {
        singleLane,
        averagePower
    };

    /**
     * One processed lane inside an analysis group
     */
    struct AnalysisLaneSpec {
        DerivedSignal signal = DerivedSignal::mid;

        AnalysisLaneSpec() = default;

        explicit AnalysisLaneSpec(DerivedSignal signalToUse)
            : signal(signalToUse) {
        }
    };

    /**
     * One published trace produced by an analysis group
     */
    struct AnalysisOutputSpec {
        TraceKind kind = TraceKind::slot1;
        OutputMixMode mixMode = OutputMixMode::singleLane;
        std::vector<size_t> laneIndices;

        AnalysisOutputSpec() = default;

        AnalysisOutputSpec(TraceKind kindToUse, OutputMixMode mixModeToUse,
                           std::initializer_list<size_t> laneIndicesToUse)
            : kind(kindToUse), mixMode(mixModeToUse), laneIndices(laneIndicesToUse) {
        }
    };

    /**
     * Processing definition for one modular analysis group
     */
    struct AnalysisGroupSpec {
        SourceFamily sourceFamily = SourceFamily::mainInput;
        std::vector<AnalysisLaneSpec> lanes;
        std::vector<AnalysisOutputSpec> outputs;

        AnalysisGroupSpec() = default;

        AnalysisGroupSpec(SourceFamily sourceFamilyToUse,
                          std::initializer_list<AnalysisLaneSpec> lanesToUse,
                          std::initializer_list<AnalysisOutputSpec> outputsToUse)
            : sourceFamily(sourceFamilyToUse), lanes(lanesToUse), outputs(outputsToUse) {
        }
    };

    /**
     * Builds the active analysis plan for the current parameter state.
     */
    class AnalysisPlanBuilder {
    public:
        std::vector<AnalysisGroupSpec> build(const EngineParameterState &parameters) const;
    };
}
