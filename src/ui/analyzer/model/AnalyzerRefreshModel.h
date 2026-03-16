#pragma once

#include <array>
#include <memory>
#include <vector>

#include "../../../dsp/core/AnalyzerData.h"
#include "../../AnalyzerDataSource.h"
#include "../../SignalSlotUiState.h"
#include "../AnalyzerUiConstants.h"
#include "../helpers/AnalyzerMeter.h"

namespace Ui {
    struct AnalyzerUiSnapshot {
        std::array<Ui::SignalSlotState, Shared::maxSignalSlots> signalSlots{};
        Shared::SignalSlotOrder signalSlotOrder{};
        Analyzer::MeterSettings meterSettings;
        float gridMinDb = 0.0f;
        float gridMaxDb = 0.0f;
        float gridStepDb = 0.0f;
    };

    struct AnalyzerRefreshDecision {
        float dtSeconds = 0.0f;
        bool uiSnapshotChanged = false;
        bool bandLayoutChanged = false;
        bool frozen = false;
        bool shouldAdvanceDisplay = false;
        bool pollingIntervalChanged = false;
        int pollIntervalMs = Ui::AnalyzerConstants::meterPollIntervalMs;
        std::shared_ptr<const std::vector<Analyzer::BandInfo>> nextBandInfo;
    };

    class AnalyzerRefreshModel final {
    public:
        void prime(const AnalyzerDataSource &dataSource);

        bool refreshUiSnapshot(const AnalyzerDataSource &dataSource, AnalyzerUiSnapshot &snapshot) const;

        bool syncFreezeEdge(const AnalyzerDataSource &dataSource,
                            Analyzer::RenderData &renderData,
                            const Analyzer::RenderData &lastPaintedRenderData);

        AnalyzerRefreshDecision makeTimerDecision(const AnalyzerDataSource &dataSource,
                                                  const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &currentBandInfo,
                                                  const AnalyzerMeter &displayMeter,
                                                  float gridMinDb,
                                                  AnalyzerUiSnapshot &snapshot);

    private:
        double lastPollTimeMs = 0.0;
        bool wasFrozen = false;
        bool isIdlePolling = false;
    };
}
