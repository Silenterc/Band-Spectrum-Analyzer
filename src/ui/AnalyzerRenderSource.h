#pragma once

#include <memory>
#include <vector>

#include "../dsp/core/AnalyzerData.h"

class AnalyzerRenderSource {
public:
    virtual ~AnalyzerRenderSource() = default;

    virtual std::shared_ptr<const std::vector<Analyzer::BandInfo>> getBandInfo() const = 0;
    virtual std::vector<Analyzer::RawTrace> getRawTraces() const = 0;
    virtual bool hasRecentSignal() const = 0;
};
