#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "display/analyzer/data/AnalyzerMeterData.h"

class AnalyzerMeter final {
public:
    void reset();

    void tick(const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo,
              const std::vector<Analyzer::RawTrace> &traces,
              bool hasNewData,
              bool advanceSilentRms,
              float hopDurationSeconds,
              const Analyzer::MeterSettings &meterSettings,
              float floorDb,
              float dtSeconds);

    const Analyzer::MeterData &getMeterData() const;
    bool isSettledAtFloor(float floorDb) const;

private:
    struct RmsHistoryEntry {
        float meanPower = 0.0f;
        float durationSeconds = 0.0f;
    };

    struct RmsWindowState {
        std::deque<RmsHistoryEntry> history;
        double weightedPowerSum = 0.0;
        double totalDurationSeconds = 0.0;
    };

    struct TraceState {
        Analyzer::TraceKind kind = Analyzer::TraceKind::slot1;
        std::vector<float> rmsDb;
        std::vector<float> peakDb;
        std::vector<float> lastPeakInputDb;
        std::vector<RmsWindowState> rmsWindows;
    };

    TraceState &getOrCreateTraceState(Analyzer::TraceKind kind, size_t bandCount, float floorDb);
    static float getPeakDb(const Analyzer::BandMeasurements &measurements, float floorDb);
    static float getHopMeanPower(const Analyzer::BandMeasurements &measurements);
    static void pushMeanPower(RmsWindowState &windowState, float meanPower, float durationSeconds, float rmsWindowMs);
    static void trimWindow(RmsWindowState &windowState, float rmsWindowMs);
    static float getWindowMeanPower(const RmsWindowState &windowState);

    std::vector<TraceState> traceStates;
    Analyzer::MeterData meterData;
    float pendingSilentRmsSeconds = 0.0f;
    float lastAppliedRmsWindowMs = -1.0f;
};
