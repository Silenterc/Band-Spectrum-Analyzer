#pragma once

#include <array>
#include <optional>

#include "display/analyzer/data/AnalyzerContributingPeakSummary.h"
#include "display/analyzer/data/AnalyzerDisplayControlState.h"
#include "display/analyzer/data/AnalyzerDisplayFrame.h"

class AnalyzerDisplayFrameModel final {
public:
    void reset();
    void syncControlState(const AnalyzerDisplayControlState &previousControlState,
                          const AnalyzerDisplayControlState &currentControlState);

    void build(const Analyzer::MeterData &liveMeterData,
               const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo,
               const AnalyzerDisplayControlState &controlState,
               float floorDb,
               AnalyzerDisplayFrame &displayFrame,
               AnalyzerContributingPeakSummary &peakSummary);

private:
    void captureFrozenTrace(size_t slotIndex);
    void clearFrozenTrace(size_t slotIndex);
    static bool isTraceCompatible(const Analyzer::MeterTrace &trace, size_t bandCount);
    static void assignSlotFrame(AnalyzerSlotDisplayFrame &slotFrame, const Analyzer::MeterTrace &trace);

    std::array<std::optional<Analyzer::MeterTrace>, Shared::maxSignalSlots> latestSlotTraces;
    std::array<std::optional<Analyzer::MeterTrace>, Shared::maxSignalSlots> frozenSlotTraces;
};
