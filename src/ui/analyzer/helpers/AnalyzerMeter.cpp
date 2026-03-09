#include "AnalyzerMeter.h"

#include <algorithm>
#include <cmath>

#include <juce_audio_basics/juce_audio_basics.h>

void AnalyzerMeter::reset() {
    traceStates.clear();
    renderData.bandInfo.clear();
    renderData.traces.clear();
}

void AnalyzerMeter::tick(const Analyzer::RawSnapshot &snapshot, const Analyzer::MeterSettings &meterSettings,
                                float floorDb, float dtSeconds) {
    renderData.bandInfo = snapshot.bandInfo;
    renderData.traces.clear();

    for (const auto &trace: snapshot.traces) {
        auto &traceState = getOrCreateTraceState(trace.kind, snapshot.bandInfo.size(), floorDb);

        Analyzer::RenderTrace renderTrace;
        renderTrace.kind = trace.kind;
        renderTrace.frame.rmsDb.resize(snapshot.bandInfo.size());
        renderTrace.frame.peakDb.resize(snapshot.bandInfo.size());
        renderTrace.frame.holdDb.resize(snapshot.bandInfo.size());

        for (size_t bandIndex = 0; bandIndex < snapshot.bandInfo.size(); ++bandIndex) {
            const auto &measurements = trace.measurements[bandIndex];
            const auto peakInputDb = getPeakDb(measurements, floorDb);
            const auto meanPower = getMeanPower(measurements);
            const auto averagedPower = pushMeanPower(traceState.rmsWindows[bandIndex], meanPower);
            const auto rmsInputDb = juce::Decibels::gainToDecibels(std::sqrt(averagedPower), floorDb);

            traceState.peakDb[bandIndex] = std::max(traceState.peakDb[bandIndex] - peakDecayDbPerSecond * dtSeconds,
                                                    peakInputDb);

            if (traceState.peakDb[bandIndex] >= traceState.holdDb[bandIndex]) {
                traceState.holdDb[bandIndex] = traceState.peakDb[bandIndex];
                traceState.holdTimeRemainingMs[bandIndex] = meterSettings.holdMs;
            } else if (traceState.holdTimeRemainingMs[bandIndex] > 0.0f) {
                traceState.holdTimeRemainingMs[bandIndex] =
                    std::max(0.0f, traceState.holdTimeRemainingMs[bandIndex] - dtSeconds * 1000.0f);
            } else {
                traceState.holdDb[bandIndex] =
                    std::max(traceState.peakDb[bandIndex], traceState.holdDb[bandIndex] - holdDecayDbPerSecond * dtSeconds);
            }

            renderTrace.frame.rmsDb[bandIndex] = meterSettings.showRms ? rmsInputDb : floorDb;
            renderTrace.frame.peakDb[bandIndex] = meterSettings.showPeak ? traceState.peakDb[bandIndex] : floorDb;
            renderTrace.frame.holdDb[bandIndex] = meterSettings.showHold ? traceState.holdDb[bandIndex] : floorDb;
        }

        renderData.traces.push_back(std::move(renderTrace));
    }
}

const Analyzer::RenderData &AnalyzerMeter::getRenderData() const {
    return renderData;
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

    traceState.peakDb.assign(bandCount, floorDb);
    traceState.holdDb.assign(bandCount, floorDb);
    traceState.holdTimeRemainingMs.assign(bandCount, 0.0f);
    traceState.rmsWindows.assign(bandCount, {});

    for (auto &windowState: traceState.rmsWindows)
        windowState.history.assign(rmsWindowPolls, 0.0);
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

float AnalyzerMeter::pushMeanPower(RmsWindowState &windowState, const float meanPower) {
    if (windowState.history.empty())
        return meanPower;

    windowState.runningSum -= windowState.history[windowState.nextIndex];
    windowState.history[windowState.nextIndex] = meanPower;
    windowState.runningSum += meanPower;
    windowState.nextIndex = (windowState.nextIndex + 1) % windowState.history.size();

    if (windowState.filled < windowState.history.size())
        ++windowState.filled;

    return static_cast<float>(windowState.runningSum / static_cast<double>(windowState.filled));
}
