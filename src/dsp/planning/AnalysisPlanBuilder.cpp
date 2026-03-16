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
            AnalysisGroupSpec spec;
            spec.kind = traceKindForSlot(slotIndex);
            spec.sourceFamily = sourceFamily;

            switch (slot.mode) {
                case SignalMode::mid:
                    spec.primarySignal = DerivedSignal::mid;
                    break;
                case SignalMode::side:
                    spec.primarySignal = DerivedSignal::side;
                    break;
                case SignalMode::stereo:
                    spec.primarySignal = DerivedSignal::left;
                    spec.secondarySignal = DerivedSignal::right;
                    break;
            }

            plan.push_back(spec);
        }

        return plan;
    }
}
