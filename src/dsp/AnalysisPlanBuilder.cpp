#include "AnalysisPlanBuilder.h"

namespace Analyzer {
    std::vector<AnalysisGroupSpec> AnalysisPlanBuilder::build(const EngineParameterState &parameters) const {
        std::vector<AnalysisGroupSpec> plan;

        plan.reserve(Shared::maxSignalSlots);

        for (size_t slotIndex = 0; slotIndex < parameters.signalSlots.size(); ++slotIndex) {
            const auto &slot = parameters.signalSlots[slotIndex];
            if (!slot.enabled)
                continue;

            const auto sourceFamily = slot.source == SignalSource::main
                                          ? SourceFamily::mainInput
                                          : SourceFamily::sidechain;
            const auto traceKind = traceKindForSlot(slotIndex);

            switch (slot.mode) {
                case SignalMode::mid:
                    plan.emplace_back(sourceFamily,
                                      std::initializer_list<AnalysisLaneSpec>{AnalysisLaneSpec(DerivedSignal::mid)},
                                      std::initializer_list<AnalysisOutputSpec>{
                                          AnalysisOutputSpec(traceKind, OutputMixMode::singleLane, {0})});
                    break;
                case SignalMode::side:
                    plan.emplace_back(sourceFamily,
                                      std::initializer_list<AnalysisLaneSpec>{AnalysisLaneSpec(DerivedSignal::side)},
                                      std::initializer_list<AnalysisOutputSpec>{
                                          AnalysisOutputSpec(traceKind, OutputMixMode::singleLane, {0})});
                    break;
                case SignalMode::stereo:
                    plan.emplace_back(sourceFamily,
                                      std::initializer_list<AnalysisLaneSpec>{
                                          AnalysisLaneSpec(DerivedSignal::left),
                                          AnalysisLaneSpec(DerivedSignal::right)},
                                      std::initializer_list<AnalysisOutputSpec>{
                                          AnalysisOutputSpec(traceKind, OutputMixMode::averagePower, {0, 1})});
                    break;
            }
        }

        return plan;
    }
}
