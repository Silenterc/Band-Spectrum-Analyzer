#include "AnalyzerRefreshModel.h"

#include <cmath>

namespace Ui {
    void AnalyzerRefreshModel::prime(const AnalyzerUiSnapshot &snapshot) {
        wasFrozen = snapshot.frozen;
        isIdlePolling = false;
        lastPollTimeMs = juce::Time::getMillisecondCounterHiRes();
    }

    bool AnalyzerRefreshModel::syncFreezeEdge(const AnalyzerUiSnapshot &snapshot,
                                              Analyzer::RenderData &renderData,
                                              const Analyzer::RenderData &lastPaintedRenderData) {
        const auto isFrozen = snapshot.frozen;
        const auto didFreezeNow = isFrozen && !wasFrozen;
        if (didFreezeNow)
            renderData = lastPaintedRenderData;

        wasFrozen = isFrozen;
        return didFreezeNow;
    }

    AnalyzerRefreshDecision AnalyzerRefreshModel::makeTimerDecision(
        const AnalyzerRenderSource &renderSource,
        const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &currentBandInfo,
        const AnalyzerMeter &displayMeter,
        const AnalyzerUiSnapshot &snapshot) {
        AnalyzerRefreshDecision decision;
        const auto currentPollTimeMs = juce::Time::getMillisecondCounterHiRes();
        decision.dtSeconds = static_cast<float>((currentPollTimeMs - lastPollTimeMs) * 0.001);
        lastPollTimeMs = currentPollTimeMs;
        decision.nextBandInfo = renderSource.getBandInfo();
        decision.bandLayoutChanged = currentBandInfo != decision.nextBandInfo;
        decision.frozen = snapshot.frozen;

        if (decision.frozen) {
            if (isIdlePolling) {
                isIdlePolling = false;
                decision.pollingIntervalChanged = true;
            }
            return decision;
        }

        decision.shouldAdvanceDisplay = renderSource.hasRecentSignal() || !displayMeter.isSettledAtFloor(snapshot.gridMinDb);
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
