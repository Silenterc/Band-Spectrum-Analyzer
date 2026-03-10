#include "AnalysisPlanBuilder.h"

namespace Analyzer {
    std::vector<AnalysisGroupSpec> AnalysisPlanBuilder::build(const ParameterState &parameters) const {
        std::vector<AnalysisGroupSpec> plan;

        switch (parameters.analysisMode) {
            case ParamSpec::AnalysisMode::mid:
                plan.emplace_back(SourceFamily::mainInput,
                                  std::initializer_list<AnalysisLaneSpec>{AnalysisLaneSpec(DerivedSignal::mid)},
                                  std::initializer_list<AnalysisOutputSpec>{
                                      AnalysisOutputSpec(TraceKind::input, OutputMixMode::singleLane, {0})});
                break;
            case ParamSpec::AnalysisMode::side:
                plan.emplace_back(SourceFamily::mainInput,
                                  std::initializer_list<AnalysisLaneSpec>{AnalysisLaneSpec(DerivedSignal::side)},
                                  std::initializer_list<AnalysisOutputSpec>{
                                      AnalysisOutputSpec(TraceKind::input, OutputMixMode::singleLane, {0})});
                break;
            case ParamSpec::AnalysisMode::stereo:
                plan.emplace_back(SourceFamily::mainInput,
                                  std::initializer_list<AnalysisLaneSpec>{
                                      AnalysisLaneSpec(DerivedSignal::left),
                                      AnalysisLaneSpec(DerivedSignal::right)},
                                  std::initializer_list<AnalysisOutputSpec>{
                                      AnalysisOutputSpec(TraceKind::input, OutputMixMode::averagePower, {0, 1})});
                break;
            case ParamSpec::AnalysisMode::sidechainMid:
                plan.emplace_back(SourceFamily::sidechain,
                                  std::initializer_list<AnalysisLaneSpec>{AnalysisLaneSpec(DerivedSignal::mid)},
                                  std::initializer_list<AnalysisOutputSpec>{
                                      AnalysisOutputSpec(TraceKind::sidechain, OutputMixMode::singleLane, {0})});
                break;
            case ParamSpec::AnalysisMode::sidechainSide:
                plan.emplace_back(SourceFamily::sidechain,
                                  std::initializer_list<AnalysisLaneSpec>{AnalysisLaneSpec(DerivedSignal::side)},
                                  std::initializer_list<AnalysisOutputSpec>{
                                      AnalysisOutputSpec(TraceKind::sidechain, OutputMixMode::singleLane, {0})});
                break;
            case ParamSpec::AnalysisMode::sidechainStereo:
                plan.emplace_back(SourceFamily::sidechain,
                                  std::initializer_list<AnalysisLaneSpec>{
                                      AnalysisLaneSpec(DerivedSignal::left),
                                      AnalysisLaneSpec(DerivedSignal::right)},
                                  std::initializer_list<AnalysisOutputSpec>{
                                      AnalysisOutputSpec(TraceKind::sidechain, OutputMixMode::averagePower, {0, 1})});
                break;
        }

        return plan;
    }
}
