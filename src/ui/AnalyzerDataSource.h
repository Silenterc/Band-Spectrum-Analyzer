#pragma once

#include "../dsp/AnalyzerData.h"
#include "analyzer/AnalyzerRenderData.h"

class AnalyzerDataSource {
public:
    virtual ~AnalyzerDataSource() = default;

    virtual Analyzer::RawSnapshot getSnapshot() const = 0;
    virtual Analyzer::MeterSettings getMeterSettings() const = 0;
    virtual float getGridMinDb() const = 0;
    virtual float getGridMaxDb() const = 0;
    virtual float getGridStepDb() const = 0;
};
