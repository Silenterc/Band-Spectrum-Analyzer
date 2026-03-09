#pragma once

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
    void tick(const Analyzer::RawSnapshot &snapshot, const Analyzer::MeterSettings &meterSettings,
              float floorDb, float dtSeconds);

    /**
     * Returns the latest meter-processed data
     */
    const Analyzer::RenderData &getRenderData() const;

private:
    struct RmsWindowState {
        // Fixed-size rectangular averaging window over recent mean-power values
        std::vector<double> history;
        // Next slot to overwrite inside the ring buffer
        size_t nextIndex = 0;
        // Number of valid values currently stored
        size_t filled = 0;
        // Running sum for O(1) rectangular averaging
        double runningSum = 0.0;
    };

    struct TraceState {
        // Identity of the trace this state belongs to
        Analyzer::TraceKind kind = Analyzer::TraceKind::input;
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
    static float pushMeanPower(RmsWindowState &windowState, float meanPower);

    static constexpr size_t rmsWindowPolls = 4;
    static constexpr float peakDecayDbPerSecond = 15.0f;
    static constexpr float holdDecayDbPerSecond = 12.0f;

    std::vector<TraceState> traceStates;
    Analyzer::RenderData renderData;
};
