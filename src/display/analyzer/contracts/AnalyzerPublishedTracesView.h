#pragma once

#include <vector>

#include "dsp/core/AnalyzerData.h"

struct AnalyzerPublishedTracesView {
    // The engine writes and the display worker reads, so this view stays single-writer/single-reader.
    const std::vector<Analyzer::RawTrace> *traces = nullptr;
    bool hasUpdate = false;

    const std::vector<Analyzer::RawTrace> &getTraces() const {
        static const std::vector<Analyzer::RawTrace> emptyTraces;
        return traces != nullptr ? *traces : emptyTraces;
    }
};
