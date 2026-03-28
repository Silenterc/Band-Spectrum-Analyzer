#include "AnalyzerMeter.h"

#include <algorithm>
#include <cmath>

#include <juce_audio_basics/juce_audio_basics.h>

void AnalyzerMeter::reset() {
    traceStates.clear();
    renderData.bandInfo.clear();
    renderData.traces.clear();
}

void AnalyzerMeter::tick(const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo,
                         const std::vector<Analyzer::RawTrace> &traces,
                         const Analyzer::MeterSettings &meterSettings,
                         float floorDb, float dtSeconds) {
    renderData.bandInfo = bandInfo != nullptr ? *bandInfo : std::vector<Analyzer::BandInfo>{};
    renderData.traces.clear();

    for (const auto &trace: traces) {
        auto &traceState = getOrCreateTraceState(trace.kind, renderData.bandInfo.size(), floorDb);

        Analyzer::RenderTrace renderTrace;
        renderTrace.kind = trace.kind;
        renderTrace.frame.rmsDb.resize(renderData.bandInfo.size());
        renderTrace.frame.peakDb.resize(renderData.bandInfo.size());
        renderTrace.frame.holdDb.resize(renderData.bandInfo.size());

        for (size_t bandIndex = 0; bandIndex < renderData.bandInfo.size(); ++bandIndex) {
            const auto &measurements = trace.measurements[bandIndex];
            const auto peakInputDb = getPeakDb(measurements, floorDb);
            const auto meanPower = getMeanPower(measurements);
            // RMS stays in the power domain until after the rectangular average
            const auto averagedPower = pushMeanPower(traceState.rmsWindows[bandIndex], meanPower, dtSeconds);
            const auto rmsInputDb = juce::Decibels::gainToDecibels(std::sqrt(averagedPower), floorDb);
            traceState.rmsDb[bandIndex] = std::max(
                traceState.rmsDb[bandIndex] - Ui::analyzerMeterTuning.rmsDecayDbPerSecond * dtSeconds,
                                                   rmsInputDb);

            // Peak jumps up immediately and only falls by a linear dB per second rate
            traceState.peakDb[bandIndex] = std::max(
                traceState.peakDb[bandIndex] - Ui::analyzerMeterTuning.peakDecayDbPerSecond * dtSeconds,
                peakInputDb);

            if (traceState.peakDb[bandIndex] >= traceState.holdDb[bandIndex]) {
                traceState.holdDb[bandIndex] = traceState.peakDb[bandIndex];
                traceState.holdTimeRemainingMs[bandIndex] = meterSettings.holdMs;
            } else if (traceState.peakDb[bandIndex] > floorDb
                       && traceState.peakDb[bandIndex]
                              >= traceState.holdDb[bandIndex] - Ui::analyzerMeterTuning.holdResetToleranceDb) {
                // Peaks within the tolerance keep the hold alive without lowering the held value.
                traceState.holdTimeRemainingMs[bandIndex] = meterSettings.holdMs;
            } else if (traceState.holdTimeRemainingMs[bandIndex] > 0.0f) {
                // Hold stays pinned until its timer runs out
                traceState.holdTimeRemainingMs[bandIndex] =
                    std::max(0.0f, traceState.holdTimeRemainingMs[bandIndex] - dtSeconds * 1000.0f);
            } else {
                // After the hold time expires it falls with its own linear dB per second rate
                traceState.holdDb[bandIndex] = std::max(
                    traceState.peakDb[bandIndex],
                    traceState.holdDb[bandIndex] - Ui::analyzerMeterTuning.holdDecayDbPerSecond * dtSeconds);
            }

            renderTrace.frame.rmsDb[bandIndex] = meterSettings.showRms ? traceState.rmsDb[bandIndex] : floorDb;
            renderTrace.frame.peakDb[bandIndex] = meterSettings.showPeak ? traceState.peakDb[bandIndex] : floorDb;
            renderTrace.frame.holdDb[bandIndex] = meterSettings.showHold ? traceState.holdDb[bandIndex] : floorDb;
        }

        renderData.traces.push_back(std::move(renderTrace));
    }
}

const Analyzer::RenderData &AnalyzerMeter::getRenderData() const {
    return renderData;
}

bool AnalyzerMeter::isSettledAtFloor(const float floorDb) const {
    for (const auto &trace: renderData.traces) {
        for (const auto value: trace.frame.rmsDb) {
            if (value > floorDb + Ui::analyzerMeterTuning.settleToleranceDb)
                return false;
        }

        for (const auto value: trace.frame.peakDb) {
            if (value > floorDb + Ui::analyzerMeterTuning.settleToleranceDb)
                return false;
        }

        for (const auto value: trace.frame.holdDb) {
            if (value > floorDb + Ui::analyzerMeterTuning.settleToleranceDb)
                return false;
        }
    }

    return true;
}

void AnalyzerMeter::ensureTraceState(Analyzer::TraceKind kind, size_t bandCount, float floorDb) {
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
    traceState.holdDb.assign(bandCount, floorDb);
    traceState.holdTimeRemainingMs.assign(bandCount, 0.0f);
    traceState.rmsWindows.assign(bandCount, {});
}

AnalyzerMeter::TraceState &AnalyzerMeter::getOrCreateTraceState(Analyzer::TraceKind kind, size_t bandCount,
                                                                              float floorDb) {
    ensureTraceState(kind, bandCount, floorDb);

    auto traceStateIterator = std::find_if(traceStates.begin(), traceStates.end(),
                                           [kind](const TraceState &traceState) { return traceState.kind == kind; });
    return *traceStateIterator;
}

float AnalyzerMeter::getPeakDb(const Analyzer::BandMeasurements &measurements, float floorDb) {
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

    // Each history entry remembers both its mean power and how long it covered
    windowState.history.push_back({meanPower, dtSeconds});
    windowState.weightedPowerSum += static_cast<double>(meanPower) * static_cast<double>(dtSeconds);
    windowState.totalDurationSeconds += dtSeconds;

    constexpr double rmsWindowSeconds = static_cast<double>(Ui::analyzerMeterTuning.rmsWindowMs) * 0.001;

    while (windowState.totalDurationSeconds > rmsWindowSeconds && !windowState.history.empty()) {
        auto &oldestEntry = windowState.history.front();
        const auto overflowSeconds = windowState.totalDurationSeconds - rmsWindowSeconds;

        if (static_cast<double>(oldestEntry.durationSeconds) <= overflowSeconds) {
            // Entire oldest slice is outside the window now, so drop it completely
            windowState.weightedPowerSum -= static_cast<double>(oldestEntry.meanPower) *
                                            static_cast<double>(oldestEntry.durationSeconds);
            windowState.totalDurationSeconds -= oldestEntry.durationSeconds;
            windowState.history.pop_front();
            continue;
        }

        // Only part of the oldest slice overflowed, so trim just that tail
        oldestEntry.durationSeconds -= static_cast<float>(overflowSeconds);
        windowState.weightedPowerSum -= static_cast<double>(oldestEntry.meanPower) * overflowSeconds;
        windowState.totalDurationSeconds = rmsWindowSeconds;
    }

    if (windowState.totalDurationSeconds <= 0.0)
        return 0.0f;

    return static_cast<float>(windowState.weightedPowerSum / windowState.totalDurationSeconds);
}
