#pragma once

#include <vector>

#include "../dsp/AnalyzerEngine.h"

class AnalyzerDataSource {
public:
    virtual ~AnalyzerDataSource() = default;

    virtual const std::vector<Analyzer::BandInfo> &getBandInfo() const = 0;
    virtual const Analyzer::Frame &getLatestFrame() const = 0;
};
