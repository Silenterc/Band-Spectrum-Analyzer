#include "display/analyzer/logic/AnalyzerGlobalHoldModel.h"

#include <algorithm>

#include "display/analyzer/data/AnalyzerDisplayTuning.h"

void AnalyzerGlobalHoldModel::reset() {
    frame.reset();
    holdTimeRemainingMs.clear();
}

void AnalyzerGlobalHoldModel::tick(const AnalyzerContributingPeakSummary &peakSummary,
                                   const Analyzer::MeterSettings &meterSettings,
                                   const float floorDb,
                                   const float dtSeconds) {
    if (!meterSettings.showHold || peakSummary.peakDb.empty()) {
        reset();
        return;
    }

    ensureFrame(peakSummary.peakDb.size(), floorDb);
    if (!frame.has_value())
        return;

    for (size_t bandIndex = 0; bandIndex < peakSummary.peakDb.size(); ++bandIndex) {
        const auto strongestPeakDb = peakSummary.peakDb[bandIndex];
        const auto strongestOwnerKind = bandIndex < peakSummary.ownerKinds.size()
                                            ? peakSummary.ownerKinds[bandIndex]
                                            : std::nullopt;
        const auto hasContributingPeak = strongestPeakDb > floorDb;

        auto &heldDb = frame->holdDb[bandIndex];
        auto &ownerKind = frame->ownerKinds[bandIndex];
        auto &holdTimeMs = holdTimeRemainingMs[bandIndex];

        if (hasContributingPeak && strongestPeakDb >= heldDb) {
            heldDb = strongestPeakDb;
            holdTimeMs = meterSettings.holdMs;
            ownerKind = strongestOwnerKind;
            continue;
        }

        const auto isWithinResetTolerance = hasContributingPeak
                                            && strongestPeakDb >= heldDb - Display::analyzerMeterTuning.holdResetToleranceDb;
        if (isWithinResetTolerance) {
            holdTimeMs = meterSettings.holdMs;
            continue;
        }

        if (holdTimeMs > 0.0f) {
            holdTimeMs = std::max(0.0f, holdTimeMs - dtSeconds * 1000.0f);
            continue;
        }

        heldDb = heldDb - meterSettings.holdDecayDbPerSecond * dtSeconds;

        if (hasContributingPeak
            && heldDb <= strongestPeakDb + Display::analyzerMeterTuning.holdResetToleranceDb) {
            heldDb = strongestPeakDb;
            holdTimeMs = meterSettings.holdMs;
            if (strongestOwnerKind.has_value())
                ownerKind = strongestOwnerKind;
        }

        if (heldDb <= floorDb + Display::analyzerMeterTuning.settleToleranceDb && !strongestOwnerKind.has_value()) {
            heldDb = floorDb;
            holdTimeMs = 0.0f;
            ownerKind.reset();
        }
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
                           return value <= floorDb + Display::analyzerMeterTuning.settleToleranceDb;
                       });
}

void AnalyzerGlobalHoldModel::ensureFrame(const size_t bandCount, const float floorDb) {
    if (!frame.has_value())
        frame = AnalyzerGlobalHoldFrame{};

    if (frame->holdDb.size() == bandCount)
        return;

    frame->holdDb.assign(bandCount, floorDb);
    frame->ownerKinds.assign(bandCount, std::nullopt);
    holdTimeRemainingMs.assign(bandCount, 0.0f);
}
