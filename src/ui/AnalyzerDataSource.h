#pragma once

#include <vector>

#include "../dsp/AnalyzerEngine.h"

class AnalyzerDataSource {
public:
    virtual ~AnalyzerDataSource() = default;

    virtual Analyzer::Snapshot getSnapshot() const = 0;
    virtual float getGridMinDb() const = 0;
    virtual float getGridMaxDb() const = 0;
    virtual float getGridStepDb() const = 0;
};
