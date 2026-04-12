#include "display/analyzer/logic/AnalyzerMeter.h"

#include <algorithm>
#include <cmath>

#include <juce_audio_basics/juce_audio_basics.h>

void AnalyzerMeter::reset() {
    traceStates.clear();
    meterData.bandInfo.reset();
    meterData.traces.clear();
}

void AnalyzerMeter::tick(const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo,
                         const std::vector<Analyzer::RawTrace> &traces,
                         const Analyzer::MeterSettings &meterSettings,
                         const float floorDb,
                         const float dtSeconds) {
    meterData.bandInfo = bandInfo;
    meterData.traces.clear();

    const auto bandCount = meterData.bandInfo != nullptr ? meterData.bandInfo->size() : 0;
    meterData.traces.reserve(traces.size());

    for (const auto &trace: traces) {
        auto &traceState = getOrCreateTraceState(trace.kind, bandCount, floorDb);

        Analyzer::MeterTrace meterTrace;
        meterTrace.kind = trace.kind;
        meterTrace.frame.rmsDb.resize(bandCount);
        meterTrace.frame.peakDb.resize(bandCount);

        for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
            const auto &measurements = trace.measurements[bandIndex];
            const auto peakInputDb = getPeakDb(measurements, floorDb);
            const auto meanPower = getMeanPower(measurements);
            const auto averagedPower = pushMeanPower(traceState.rmsWindows[bandIndex], meanPower, dtSeconds);
            const auto rmsInputDb = juce::Decibels::gainToDecibels(std::sqrt(averagedPower), floorDb);
            traceState.rmsDb[bandIndex] = std::max(
                traceState.rmsDb[bandIndex] - Ui::analyzerMeterTuning.rmsDecayDbPerSecond * dtSeconds,
                rmsInputDb);
            traceState.peakDb[bandIndex] = std::max(
                traceState.peakDb[bandIndex] - Ui::analyzerMeterTuning.peakDecayDbPerSecond * dtSeconds,
                peakInputDb);

            meterTrace.frame.rmsDb[bandIndex] = meterSettings.showRms ? traceState.rmsDb[bandIndex] : floorDb;
            meterTrace.frame.peakDb[bandIndex] = meterSettings.showPeak ? traceState.peakDb[bandIndex] : floorDb;
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
    traceState.peakDb.assign(bandCount, floorDb);
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

float AnalyzerMeter::getMeanPower(const Analyzer::BandMeasurements &measurements) {
    if (measurements.numSamples <= 0)
        return 0.0f;

    return static_cast<float>(measurements.sumPower / static_cast<double>(measurements.numSamples));
}

float AnalyzerMeter::pushMeanPower(RmsWindowState &windowState, const float meanPower, const float dtSeconds) {
    if (dtSeconds <= 0.0f)
        return meanPower;

    windowState.history.push_back({meanPower, dtSeconds});
    windowState.weightedPowerSum += static_cast<double>(meanPower) * static_cast<double>(dtSeconds);
    windowState.totalDurationSeconds += dtSeconds;

    constexpr double rmsWindowSeconds = static_cast<double>(Ui::analyzerMeterTuning.rmsWindowMs) * 0.001;

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

    if (windowState.totalDurationSeconds <= 0.0)
        return 0.0f;

    return static_cast<float>(windowState.weightedPowerSum / windowState.totalDurationSeconds);
}
