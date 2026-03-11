#pragma once

#include <memory>
#include <vector>

#include "../dsp/AnalyzerData.h"
#include "SignalSlotUiState.h"
#include "analyzer/AnalyzerRenderData.h"

class AnalyzerDataSource {
public:
    virtual ~AnalyzerDataSource() = default;

    virtual std::shared_ptr<const std::vector<Analyzer::BandInfo>> getBandInfo() const = 0;
    virtual std::vector<Analyzer::RawTrace> getRawTraces() const = 0;
    virtual std::array<Ui::SignalSlotState, Shared::maxSignalSlots> getSignalSlots() const = 0;
    virtual Shared::SignalSlotOrder getSignalSlotOrder() const = 0;
    virtual bool isSidechainAvailable() const = 0;
    virtual bool isFrozen() const = 0;
    virtual Analyzer::MeterSettings getMeterSettings() const = 0;
    virtual float getGridMinDb() const = 0;
    virtual float getGridMaxDb() const = 0;
    virtual float getGridStepDb() const = 0;
};
