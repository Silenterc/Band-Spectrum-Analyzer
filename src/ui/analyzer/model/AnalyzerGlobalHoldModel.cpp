#include "AnalyzerGlobalHoldModel.h"

#include <algorithm>

#include "AnalyzerMeterTuning.h"
#include "AnalyzerUiSelectors.h"

void AnalyzerGlobalHoldModel::reset() {
    frame.reset();
    holdTimeRemainingMs.clear();
}

void AnalyzerGlobalHoldModel::tick(const Analyzer::RenderData &renderData,
                                   const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                   const Analyzer::MeterSettings &meterSettings,
                                   const float floorDb,
                                   const float dtSeconds) {
    if (!meterSettings.showHold || renderData.bandInfo.empty()) {
        reset();
        return;
    }

    ensureFrame(renderData.bandInfo.size(), floorDb);
    if (!frame.has_value())
        return;

    for (size_t bandIndex = 0; bandIndex < renderData.bandInfo.size(); ++bandIndex) {
        auto strongestPeakDb = floorDb;
        std::optional<Analyzer::TraceKind> strongestOwnerKind;

        for (const auto &trace: renderData.traces) {
            if (!Ui::isTraceVisible(trace.kind, signalSlots))
                continue;

            if (bandIndex >= trace.frame.peakDb.size())
                continue;

            if (trace.frame.peakDb[bandIndex] > strongestPeakDb) {
                strongestPeakDb = trace.frame.peakDb[bandIndex];
                strongestOwnerKind = trace.kind;
            }
        }

        auto &heldDb = frame->holdDb[bandIndex];
        auto &ownerKind = frame->ownerKinds[bandIndex];
        auto &holdTimeMs = holdTimeRemainingMs[bandIndex];

        if (strongestPeakDb >= heldDb) {
            heldDb = strongestPeakDb;
            holdTimeMs = meterSettings.holdMs;
            ownerKind = strongestOwnerKind;
            continue;
        }

        if (strongestPeakDb > floorDb
            && strongestPeakDb >= heldDb - Ui::analyzerMeterTuning.holdResetToleranceDb) {
            holdTimeMs = meterSettings.holdMs;
            continue;
        }

        if (holdTimeMs > 0.0f) {
            holdTimeMs = std::max(0.0f, holdTimeMs - dtSeconds * 1000.0f);
            continue;
        }

        const auto decayedDb = heldDb - Ui::analyzerMeterTuning.holdDecayDbPerSecond * dtSeconds;
        if (strongestPeakDb >= decayedDb) {
            heldDb = strongestPeakDb;
            if (strongestOwnerKind.has_value())
                ownerKind = strongestOwnerKind;
        } else {
            heldDb = decayedDb;
        }

        if (heldDb <= floorDb + Ui::analyzerMeterTuning.settleToleranceDb && !strongestOwnerKind.has_value())
            ownerKind.reset();
    }
}

const std::optional<AnalyzerGlobalHoldFrame> &AnalyzerGlobalHoldModel::getFrame() const {
    return frame;
}

bool AnalyzerGlobalHoldModel::isSettledAtFloor(const float floorDb) const {
    if (!frame.has_value())
        return true;

    return std::all_of(frame->holdDb.begin(), frame->holdDb.end(),
                       [floorDb](const float value) {
                           return value <= floorDb + Ui::analyzerMeterTuning.settleToleranceDb;
                       });
}

void AnalyzerGlobalHoldModel::ensureFrame(const size_t bandCount, const float floorDb) {
    if (!frame.has_value()) {
        frame = AnalyzerGlobalHoldFrame{};
    }

    if (frame->holdDb.size() == bandCount)
        return;

    frame->holdDb.assign(bandCount, floorDb);
    frame->ownerKinds.assign(bandCount, std::nullopt);
    holdTimeRemainingMs.assign(bandCount, 0.0f);
}
