#pragma once

#include <optional>

#include <juce_core/juce_core.h>

#include "dsp/core/AnalyzerRawTraceSource.h"
#include "display/analyzer/data/AnalyzerDisplayControlState.h"
#include "display/analyzer/logic/AnalyzerDisplayFrameModel.h"
#include "display/analyzer/logic/AnalyzerGlobalHoldModel.h"
#include "display/analyzer/logic/AnalyzerMeter.h"
#include "display/analyzer/thread/AnalyzerDisplayFrameBuffer.h"

class AnalyzerDisplayWorker final : private juce::Thread {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void analyzerDisplayFramePublished() = 0;
    };

    AnalyzerDisplayWorker(AnalyzerRawTraceSource &rawTraceSource, Listener &listener);
    ~AnalyzerDisplayWorker() override;

    void start();
    void stop();
    void setControlState(const AnalyzerDisplayControlState &controlState);
    const AnalyzerDisplayFrame *readLatestFrame(bool &hasUpdate);

private:
    void run() override;
    void computeFrame(float dtSeconds);
    void resetDisplayModels();

    AnalyzerRawTraceSource &rawTraceSource;
    Listener &listener;
    AnalyzerMeter meter;
    AnalyzerDisplayFrameModel displayFrameModel;
    AnalyzerGlobalHoldModel globalHoldModel;
    AnalyzerDisplayFrameBuffer frameBuffer;
    juce::WaitableEvent wakeEvent;
    juce::CriticalSection controlLock;
    AnalyzerDisplayControlState controlState;
    AnalyzerDisplayControlState previousControlState;
    std::shared_ptr<const std::vector<Analyzer::BandInfo>> currentBandInfo;
    std::uint64_t nextRevision = 1;
    double lastTickTimeMs = 0.0;
    bool forceRefresh = true;
    bool useIdlePolling = false;
};
