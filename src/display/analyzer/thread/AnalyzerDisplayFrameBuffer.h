#pragma once

#include "display/analyzer/data/AnalyzerDisplayFrame.h"
#include "dsp/util/TripleBuffer.h"

class AnalyzerDisplayFrameBuffer final {
public:
    AnalyzerDisplayFrame &getForWriter() {
        return *buffer.get_for_writer();
    }

    void publish() {
        buffer.publish();
    }

    const AnalyzerDisplayFrame *getForReader(bool &hasUpdate) {
        const auto [frame, updated] = buffer.get_for_reader();
        hasUpdate = updated;
        return frame;
    }

private:
    TripleBuffer<AnalyzerDisplayFrame> buffer;
};
