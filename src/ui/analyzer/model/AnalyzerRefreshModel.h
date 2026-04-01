#pragma once

#include <array>
#include <memory>
#include <vector>

#include "../../../dsp/core/AnalyzerData.h"
#include "../../AnalyzerRenderSource.h"
#include "../AnalyzerUiConstants.h"
#include "AnalyzerUiSnapshot.h"
#include "../helpers/AnalyzerMeter.h"
#include "AnalyzerGlobalHoldModel.h"

namespace Ui {
    struct AnalyzerRefreshDecision {
        float dtSeconds = 0.0f;
        bool bandLayoutChanged = false;
        bool frozen = false;
        bool shouldAdvanceDisplay = false;
        bool pollingIntervalChanged = false;
        int pollIntervalMs = Ui::AnalyzerConstants::meterPollIntervalMs;
        std::shared_ptr<const std::vector<Analyzer::BandInfo>> nextBandInfo;
    };

    class AnalyzerRefreshModel final {
    public:
        void prime(const AnalyzerUiSnapshot &snapshot);

        bool syncFreezeEdge(const AnalyzerUiSnapshot &snapshot,
                            Analyzer::RenderData &renderData,
                            const Analyzer::RenderData &lastPaintedRenderData);

        AnalyzerRefreshDecision makeTimerDecision(const AnalyzerRenderSource &renderSource,
                                                  const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &currentBandInfo,
                                                  const AnalyzerMeter &displayMeter,
                                                  const AnalyzerGlobalHoldModel &globalHoldModel,
                                                  const AnalyzerUiSnapshot &snapshot);

    private:
        double lastPollTimeMs = 0.0;
        bool wasFrozen = false;
        bool isIdlePolling = false;
    };
}
