#include "AnalyzerRefreshModel.h"

#include <cmath>

namespace {
    bool nearlyEqual(const float lhs, const float rhs) {
        return std::abs(lhs - rhs) <= 0.0001f;
    }
}

namespace Ui {
    void AnalyzerRefreshModel::prime(const AnalyzerDataSource &dataSource) {
        wasFrozen = dataSource.isFrozen();
        isIdlePolling = false;
        lastPollTimeMs = juce::Time::getMillisecondCounterHiRes();
    }

    bool AnalyzerRefreshModel::refreshUiSnapshot(const AnalyzerDataSource &dataSource, AnalyzerUiSnapshot &snapshot) const {
        const auto previousSignalSlots = snapshot.signalSlots;
        const auto previousSignalSlotOrder = snapshot.signalSlotOrder;
        const auto previousMeterSettings = snapshot.meterSettings;
        const auto previousGridMinDb = snapshot.gridMinDb;
        const auto previousGridMaxDb = snapshot.gridMaxDb;
        const auto previousGridStepDb = snapshot.gridStepDb;

        snapshot.signalSlots = dataSource.getSignalSlots();
        snapshot.signalSlotOrder = dataSource.getSignalSlotOrder();
        snapshot.meterSettings = dataSource.getMeterSettings();
        snapshot.gridMinDb = dataSource.getGridMinDb();
        snapshot.gridMaxDb = dataSource.getGridMaxDb();
        snapshot.gridStepDb = dataSource.getGridStepDb();

        return snapshot.signalSlots != previousSignalSlots
               || snapshot.signalSlotOrder != previousSignalSlotOrder
               || snapshot.meterSettings.showRms != previousMeterSettings.showRms
               || snapshot.meterSettings.showPeak != previousMeterSettings.showPeak
               || snapshot.meterSettings.showHold != previousMeterSettings.showHold
               || !nearlyEqual(snapshot.meterSettings.holdMs, previousMeterSettings.holdMs)
               || !nearlyEqual(snapshot.gridMinDb, previousGridMinDb)
               || !nearlyEqual(snapshot.gridMaxDb, previousGridMaxDb)
               || !nearlyEqual(snapshot.gridStepDb, previousGridStepDb);
    }

    bool AnalyzerRefreshModel::syncFreezeEdge(const AnalyzerDataSource &dataSource,
                                              Analyzer::RenderData &renderData,
                                              const Analyzer::RenderData &lastPaintedRenderData) {
        const auto isFrozen = dataSource.isFrozen();
        const auto didFreezeNow = isFrozen && !wasFrozen;
        if (didFreezeNow)
            renderData = lastPaintedRenderData;

        wasFrozen = isFrozen;
        return didFreezeNow;
    }

    AnalyzerRefreshDecision AnalyzerRefreshModel::makeTimerDecision(
        const AnalyzerDataSource &dataSource,
        const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &currentBandInfo,
        const AnalyzerMeter &displayMeter,
        const float gridMinDb,
        AnalyzerUiSnapshot &snapshot) {
        AnalyzerRefreshDecision decision;
        const auto currentPollTimeMs = juce::Time::getMillisecondCounterHiRes();
        decision.dtSeconds = static_cast<float>((currentPollTimeMs - lastPollTimeMs) * 0.001);
        lastPollTimeMs = currentPollTimeMs;
        decision.uiSnapshotChanged = refreshUiSnapshot(dataSource, snapshot);
        decision.nextBandInfo = dataSource.getBandInfo();
        decision.bandLayoutChanged = currentBandInfo != decision.nextBandInfo;
        decision.frozen = dataSource.isFrozen();

        if (decision.frozen) {
            if (isIdlePolling) {
                isIdlePolling = false;
                decision.pollingIntervalChanged = true;
            }
            return decision;
        }

        decision.shouldAdvanceDisplay = dataSource.hasRecentSignal() || !displayMeter.isSettledAtFloor(gridMinDb);
        const auto shouldUseIdlePolling = !decision.shouldAdvanceDisplay;
        if (isIdlePolling != shouldUseIdlePolling) {
            isIdlePolling = shouldUseIdlePolling;
            decision.pollingIntervalChanged = true;
        }

        decision.pollIntervalMs = isIdlePolling ? Ui::AnalyzerConstants::idleMeterPollIntervalMs
                                                : Ui::AnalyzerConstants::meterPollIntervalMs;
        return decision;
    }
}
