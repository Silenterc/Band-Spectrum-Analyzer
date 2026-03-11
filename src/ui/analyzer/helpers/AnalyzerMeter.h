#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "../../../dsp/AnalyzerData.h"
#include "../AnalyzerRenderData.h"

/**
 * Converts raw engine measurements into display-rate RMS, peak, and hold values
 */
class AnalyzerMeter final {
public:
    /**
     * Clears all display state
     */
    void reset();

    /**
     * Advances the display meter by one UI poll
     */
    void tick(const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo,
              const std::vector<Analyzer::RawTrace> &traces,
              const Analyzer::MeterSettings &meterSettings, float floorDb, float dtSeconds);

    /**
     * Returns the latest meter-processed data
     */
    const Analyzer::RenderData &getRenderData() const;

private:
    struct RmsHistoryEntry {
        // Mean power measured over one meter tick
        float meanPower = 0.0f;
        // How long that measurement covered
        float durationSeconds = 0.0f;
    };

    struct RmsWindowState {
        // Recent mean-power measurements kept inside the RMS time window
        std::deque<RmsHistoryEntry> history;
        // Running weighted sum of meanPower * durationSeconds
        double weightedPowerSum = 0.0;
        // Total time currently covered by the history
        double totalDurationSeconds = 0.0;
    };

    struct TraceState {
        // Identity of the trace this state belongs to
        Analyzer::TraceKind kind = Analyzer::TraceKind::slot1;
        // Displayed peak values after linear dB/s decay
        std::vector<float> peakDb;
        // Displayed hold values
        std::vector<float> holdDb;
        // Hold timers in milliseconds
        std::vector<float> holdTimeRemainingMs;
        // Rectangular RMS averaging state per band
        std::vector<RmsWindowState> rmsWindows;
    };

    void ensureTraceState(Analyzer::TraceKind kind, size_t bandCount, float floorDb);
    TraceState &getOrCreateTraceState(Analyzer::TraceKind kind, size_t bandCount, float floorDb);
    static float getPeakDb(const Analyzer::BandMeasurements &measurements, float floorDb);
    static float getMeanPower(const Analyzer::BandMeasurements &measurements);
    static float pushMeanPower(RmsWindowState &windowState, float meanPower, float dtSeconds);

    static constexpr float rmsWindowMs = 180.0f;
    static constexpr float peakDecayDbPerSecond = 15.0f;
    static constexpr float holdDecayDbPerSecond = 12.0f;

    std::vector<TraceState> traceStates;
    Analyzer::RenderData renderData;
};
