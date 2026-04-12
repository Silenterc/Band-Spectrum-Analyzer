#pragma once

#include <memory>
#include <vector>

#include "display/analyzer/contracts/AnalyzerPublishedTracesView.h"
#include "dsp/core/AnalyzerData.h"

class AnalyzerRawTraceSource {
public:
    virtual ~AnalyzerRawTraceSource() = default;

    virtual std::shared_ptr<const std::vector<Analyzer::BandInfo>> getBandInfo() const = 0;
    virtual AnalyzerPublishedTracesView readPublishedTraces() const = 0;
    virtual bool hasRecentSignal() const = 0;
};
