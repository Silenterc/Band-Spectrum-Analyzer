#pragma once

#include "../dsp/AnalyzerData.h"

class AnalyzerDataSource {
public:
    virtual ~AnalyzerDataSource() = default;

    virtual Analyzer::CompositeSnapshot getSnapshot() const = 0;
    virtual float getGridMinDb() const = 0;
    virtual float getGridMaxDb() const = 0;
    virtual float getGridStepDb() const = 0;
};
