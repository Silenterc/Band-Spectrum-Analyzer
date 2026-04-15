#include "display/analyzer/logic/AnalyzerMeter.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <juce_audio_basics/juce_audio_basics.h>

namespace {
    constexpr float discardedPeakDb = -std::numeric_limits<float>::infinity();

    float snapRmsToDisplayFloor(const float valueDb, const float floorDb) {
        return valueDb <= floorDb + Ui::analyzerMeterTuning.settleToleranceDb ? floorDb : valueDb;
    }

    void discardPeakIfBelowFloor(float &peakDb, float &lastPeakInputDb, const float floorDb) {
        if (peakDb > floorDb + Ui::analyzerMeterTuning.settleToleranceDb)
            return;

        peakDb = discardedPeakDb;
        lastPeakInputDb = discardedPeakDb;
    }
}

void AnalyzerMeter::reset() {
    traceStates.clear();
    meterData.bandInfo.reset();
    meterData.traces.clear();
    pendingSilentRmsSeconds = 0.0f;
    lastAppliedRmsWindowMs = -1.0f;
}

void AnalyzerMeter::tick(const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo,
                         const std::vector<Analyzer::RawTrace> &traces,
                         const bool hasNewData,
                         const bool advanceSilentRms,
                         const float hopDurationSeconds,
                         const Analyzer::MeterSettings &meterSettings,
                         const float floorDb,
                         const float dtSeconds) {
    meterData.bandInfo = bandInfo;
    meterData.traces.clear();

    const auto bandCount = meterData.bandInfo != nullptr ? meterData.bandInfo->size() : 0;
    meterData.traces.reserve(traces.size());
    const auto rmsWindowChanged = !juce::approximatelyEqual(lastAppliedRmsWindowMs, meterSettings.rmsWindowMs);
    lastAppliedRmsWindowMs = meterSettings.rmsWindowMs;

    if (hasNewData) {
        pendingSilentRmsSeconds = 0.0f;
    } else if (advanceSilentRms) {
        pendingSilentRmsSeconds += dtSeconds;
    } else {
        pendingSilentRmsSeconds = 0.0f;
    }

    int silentHopCount = 0;
    if (!hasNewData && advanceSilentRms && hopDurationSeconds > 0.0f) {
        while (pendingSilentRmsSeconds >= hopDurationSeconds) {
            ++silentHopCount;
            pendingSilentRmsSeconds -= hopDurationSeconds;
        }
    }

    for (const auto &trace: traces) {
        auto &traceState = getOrCreateTraceState(trace.kind, bandCount, floorDb);

        Analyzer::MeterTrace meterTrace;
        meterTrace.kind = trace.kind;
        meterTrace.frame.rmsDb.resize(bandCount);
        meterTrace.frame.peakDb.resize(bandCount);

        for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
            if (hasNewData && bandIndex < trace.measurements.size()) {
                const auto &measurements = trace.measurements[bandIndex];
                traceState.lastPeakInputDb[bandIndex] = getPeakDb(measurements, floorDb);
                if (hopDurationSeconds > 0.0f) {
                    const auto meanPower = getHopMeanPower(measurements);
                    pushMeanPower(traceState.rmsWindows[bandIndex],
                                  meanPower,
                                  hopDurationSeconds,
                                  meterSettings.rmsWindowMs);
                }
            } else if (silentHopCount > 0 && hopDurationSeconds > 0.0f) {
                for (int silentHopIndex = 0; silentHopIndex < silentHopCount; ++silentHopIndex) {
                    pushMeanPower(traceState.rmsWindows[bandIndex],
                                  0.0f,
                                  hopDurationSeconds,
                                  meterSettings.rmsWindowMs);
                }
            }

            if (rmsWindowChanged)
                trimWindow(traceState.rmsWindows[bandIndex], meterSettings.rmsWindowMs);

            traceState.peakDb[bandIndex] = std::max(
                traceState.peakDb[bandIndex] - Ui::analyzerMeterTuning.peakDecayDbPerSecond * dtSeconds,
                traceState.lastPeakInputDb[bandIndex]);
            traceState.rmsDb[bandIndex] = juce::Decibels::gainToDecibels(
                std::sqrt(getWindowMeanPower(traceState.rmsWindows[bandIndex])),
                floorDb);
            discardPeakIfBelowFloor(traceState.peakDb[bandIndex], traceState.lastPeakInputDb[bandIndex], floorDb);
            traceState.rmsDb[bandIndex] = snapRmsToDisplayFloor(traceState.rmsDb[bandIndex], floorDb);

            meterTrace.frame.rmsDb[bandIndex] = meterSettings.showRms ? traceState.rmsDb[bandIndex] : floorDb;
            meterTrace.frame.peakDb[bandIndex] = meterSettings.showPeak
                                                     ? (std::isfinite(traceState.peakDb[bandIndex])
                                                            ? traceState.peakDb[bandIndex]
                                                            : floorDb)
                                                     : floorDb;
        }

        meterData.traces.push_back(std::move(meterTrace));
    }
}

const Analyzer::MeterData &AnalyzerMeter::getMeterData() const {
    return meterData;
}

bool AnalyzerMeter::isSettledAtFloor(const float floorDb) const {
    for (const auto &trace: meterData.traces) {
        for (const auto value: trace.frame.rmsDb) {
            if (value > floorDb + Ui::analyzerMeterTuning.settleToleranceDb)
                return false;
        }

        for (const auto value: trace.frame.peakDb) {
            if (value > floorDb + Ui::analyzerMeterTuning.settleToleranceDb)
                return false;
        }
    }

    return true;
}

void AnalyzerMeter::ensureTraceState(const Analyzer::TraceKind kind, const size_t bandCount, const float floorDb) {
    auto traceStateIterator = std::find_if(traceStates.begin(), traceStates.end(),
                                           [kind](const TraceState &traceState) { return traceState.kind == kind; });

    if (traceStateIterator == traceStates.end()) {
        TraceState traceState;
        traceState.kind = kind;
        traceStates.push_back(std::move(traceState));
        traceStateIterator = std::prev(traceStates.end());
    }

    auto &traceState = *traceStateIterator;
    if (traceState.peakDb.size() == bandCount)
        return;

    traceState.rmsDb.assign(bandCount, floorDb);
    traceState.peakDb.assign(bandCount, discardedPeakDb);
    traceState.lastPeakInputDb.assign(bandCount, discardedPeakDb);
    traceState.rmsWindows.assign(bandCount, {});
}

AnalyzerMeter::TraceState &AnalyzerMeter::getOrCreateTraceState(const Analyzer::TraceKind kind,
                                                                const size_t bandCount,
                                                                const float floorDb) {
    ensureTraceState(kind, bandCount, floorDb);

    const auto traceStateIterator = std::find_if(traceStates.begin(), traceStates.end(),
                                                 [kind](const TraceState &traceState) {
                                                     return traceState.kind == kind;
                                                 });
    return *traceStateIterator;
}

float AnalyzerMeter::getPeakDb(const Analyzer::BandMeasurements &measurements, const float floorDb) {
    return juce::Decibels::gainToDecibels(std::sqrt(measurements.peakPower), floorDb);
}

float AnalyzerMeter::getHopMeanPower(const Analyzer::BandMeasurements &measurements) {
    if (measurements.rmsHopNumSamples <= 0)
        return 0.0f;

    return static_cast<float>(measurements.rmsHopSumPower / static_cast<double>(measurements.rmsHopNumSamples));
}

void AnalyzerMeter::pushMeanPower(RmsWindowState &windowState,
                                  const float meanPower,
                                  const float durationSeconds,
                                  const float rmsWindowMs) {
    if (durationSeconds <= 0.0f)
        return;

    windowState.history.push_back({meanPower, durationSeconds});
    windowState.weightedPowerSum += static_cast<double>(meanPower) * static_cast<double>(durationSeconds);
    windowState.totalDurationSeconds += durationSeconds;

    trimWindow(windowState, rmsWindowMs);
}

void AnalyzerMeter::trimWindow(RmsWindowState &windowState, const float rmsWindowMs) {
    const auto rmsWindowSeconds = std::max(0.0, static_cast<double>(rmsWindowMs) * 0.001);

    while (windowState.totalDurationSeconds > rmsWindowSeconds && !windowState.history.empty()) {
        auto &oldestEntry = windowState.history.front();
        const auto overflowSeconds = windowState.totalDurationSeconds - rmsWindowSeconds;

        if (static_cast<double>(oldestEntry.durationSeconds) <= overflowSeconds) {
            windowState.weightedPowerSum -= static_cast<double>(oldestEntry.meanPower)
                                            * static_cast<double>(oldestEntry.durationSeconds);
            windowState.totalDurationSeconds -= oldestEntry.durationSeconds;
            windowState.history.pop_front();
            continue;
        }

        oldestEntry.durationSeconds -= static_cast<float>(overflowSeconds);
        windowState.weightedPowerSum -= static_cast<double>(oldestEntry.meanPower) * overflowSeconds;
        windowState.totalDurationSeconds = rmsWindowSeconds;
    }
}

float AnalyzerMeter::getWindowMeanPower(const RmsWindowState &windowState) {
    if (windowState.totalDurationSeconds <= 0.0)
        return 0.0f;

    return static_cast<float>(windowState.weightedPowerSum / windowState.totalDurationSeconds);
}
