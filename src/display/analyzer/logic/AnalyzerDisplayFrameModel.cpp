#include "display/analyzer/logic/AnalyzerDisplayFrameModel.h"

void AnalyzerDisplayFrameModel::reset() {
    latestSlotTraces.fill(std::nullopt);
    frozenSlotTraces.fill(std::nullopt);
}

void AnalyzerDisplayFrameModel::syncControlState(const AnalyzerDisplayControlState &previousControlState,
                                                 const AnalyzerDisplayControlState &currentControlState) {
    for (size_t slotIndex = 0; slotIndex < currentControlState.slotFrozen.size(); ++slotIndex) {
        if (!currentControlState.slotFrozen[slotIndex]) {
            clearFrozenTrace(slotIndex);
            continue;
        }

        if (!previousControlState.slotFrozen[slotIndex] && currentControlState.slotFrozen[slotIndex])
            captureFrozenTrace(slotIndex);
    }
}

void AnalyzerDisplayFrameModel::build(const Analyzer::MeterData &liveMeterData,
                                      const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo,
                                      const AnalyzerDisplayControlState &controlState,
                                      const float floorDb,
                                      AnalyzerDisplayFrame &displayFrame,
                                      AnalyzerContributingPeakSummary &peakSummary) {
    const auto bandCount = bandInfo != nullptr ? bandInfo->size() : 0;
    displayFrame.bandInfo = bandInfo;

    for (size_t slotIndex = 0; slotIndex < displayFrame.slotFrames.size(); ++slotIndex) {
        auto &slotFrame = displayFrame.slotFrames[slotIndex];
        slotFrame.active = false;
        slotFrame.kind = Analyzer::traceKindForSlot(slotIndex);

        if (!controlState.slotFrozen[slotIndex])
            clearFrozenTrace(slotIndex);
    }

    std::array<bool, Shared::maxSignalSlots> hasLiveTrace{};

    for (const auto &liveTrace: liveMeterData.traces) {
        const auto slotIndex = Analyzer::slotIndexForTraceKind(liveTrace.kind);
        if (slotIndex >= displayFrame.slotFrames.size())
            continue;

        hasLiveTrace[slotIndex] = true;
        latestSlotTraces[slotIndex] = liveTrace;
        const Analyzer::MeterTrace *displayTrace = &liveTrace;

        if (controlState.slotFrozen[slotIndex]) {
            if (!frozenSlotTraces[slotIndex].has_value()
                || !isTraceCompatible(*frozenSlotTraces[slotIndex], bandCount)) {
                frozenSlotTraces[slotIndex] = liveTrace;
            }
            displayTrace = &*frozenSlotTraces[slotIndex];
        }

        assignSlotFrame(displayFrame.slotFrames[slotIndex], *displayTrace);
    }

    for (size_t slotIndex = 0; slotIndex < controlState.slotFrozen.size(); ++slotIndex) {
        if (!controlState.slotFrozen[slotIndex] || hasLiveTrace[slotIndex])
            continue;

        if (!frozenSlotTraces[slotIndex].has_value()
            || !isTraceCompatible(*frozenSlotTraces[slotIndex], bandCount)) {
            continue;
        }

        assignSlotFrame(displayFrame.slotFrames[slotIndex], *frozenSlotTraces[slotIndex]);
    }

    peakSummary.peakDb.assign(bandCount, floorDb);
    peakSummary.ownerKinds.assign(bandCount, std::nullopt);

    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        for (size_t slotIndex = 0; slotIndex < displayFrame.slotFrames.size(); ++slotIndex) {
            if (!controlState.slotContributing[slotIndex])
                continue;

            const auto &slotFrame = displayFrame.slotFrames[slotIndex];
            if (!slotFrame.active || bandIndex >= slotFrame.frame.peakDb.size())
                continue;

            if (slotFrame.frame.peakDb[bandIndex] > peakSummary.peakDb[bandIndex]) {
                peakSummary.peakDb[bandIndex] = slotFrame.frame.peakDb[bandIndex];
                peakSummary.ownerKinds[bandIndex] = slotFrame.kind;
            }
        }
    }
}

void AnalyzerDisplayFrameModel::captureFrozenTrace(const size_t slotIndex) {
    if (slotIndex >= frozenSlotTraces.size())
        return;

    if (latestSlotTraces[slotIndex].has_value())
        frozenSlotTraces[slotIndex] = latestSlotTraces[slotIndex];
}

void AnalyzerDisplayFrameModel::clearFrozenTrace(const size_t slotIndex) {
    if (slotIndex >= frozenSlotTraces.size())
        return;

    frozenSlotTraces[slotIndex].reset();
}

bool AnalyzerDisplayFrameModel::isTraceCompatible(const Analyzer::MeterTrace &trace, const size_t bandCount) {
    return trace.frame.rmsDb.size() == bandCount
           && trace.frame.peakDb.size() == bandCount;
}

void AnalyzerDisplayFrameModel::assignSlotFrame(AnalyzerSlotDisplayFrame &slotFrame,
                                                const Analyzer::MeterTrace &trace) {
    slotFrame.active = true;
    slotFrame.kind = trace.kind;
    slotFrame.frame = trace.frame;
}
