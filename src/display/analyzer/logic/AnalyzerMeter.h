#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "display/analyzer/data/AnalyzerMeterData.h"
#include "ui/analyzer/plot/logic/AnalyzerMeterTuning.h"

class AnalyzerMeter final {
public:
    void reset();

    void tick(const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo,
              const std::vector<Analyzer::RawTrace> &traces,
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
        std::vector<RmsWindowState> rmsWindows;
    };

    void ensureTraceState(Analyzer::TraceKind kind, size_t bandCount, float floorDb);
    TraceState &getOrCreateTraceState(Analyzer::TraceKind kind, size_t bandCount, float floorDb);
    static float getPeakDb(const Analyzer::BandMeasurements &measurements, float floorDb);
    static float getMeanPower(const Analyzer::BandMeasurements &measurements);
    static float pushMeanPower(RmsWindowState &windowState, float meanPower, float dtSeconds);

    std::vector<TraceState> traceStates;
    Analyzer::MeterData meterData;
};
