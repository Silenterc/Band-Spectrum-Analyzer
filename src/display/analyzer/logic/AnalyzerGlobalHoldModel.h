#pragma once

#include <optional>
#include <vector>

#include "display/analyzer/data/AnalyzerContributingPeakSummary.h"
#include "display/analyzer/data/AnalyzerGlobalHoldFrame.h"
#include "display/analyzer/data/AnalyzerMeterData.h"

class AnalyzerGlobalHoldModel final {
public:
    void reset();

    void tick(const AnalyzerContributingPeakSummary &peakSummary,
              const Analyzer::MeterSettings &meterSettings,
              float floorDb,
              float dtSeconds);

    const std::optional<AnalyzerGlobalHoldFrame> &getFrame() const;
    bool isSettledAtFloor(float floorDb) const;

private:
    void ensureFrame(size_t bandCount, float floorDb);

    std::optional<AnalyzerGlobalHoldFrame> frame;
    std::vector<float> holdTimeRemainingMs;
};
