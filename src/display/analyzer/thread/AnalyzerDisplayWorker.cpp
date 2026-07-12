#include "display/analyzer/thread/AnalyzerDisplayWorker.h"

#include "display/analyzer/config/AnalyzerDisplayConstants.h"
#include "display/analyzer/thread/AnalyzerDisplayTickScheduler.h"

namespace {
    const std::vector<Analyzer::RawTrace>& emptyRawTraces() {
        static const std::vector<Analyzer::RawTrace> traces;
        return traces;
    }
}

AnalyzerDisplayWorker::AnalyzerDisplayWorker(AnalyzerRawTraceSource &rawTraceSourceToUse,
                                             Listener &listenerToUse)
    : juce::Thread("AnalyzerDisplayWorker"),
      rawTraceSource(rawTraceSourceToUse),
      listener(listenerToUse) {
}

AnalyzerDisplayWorker::~AnalyzerDisplayWorker() {
    stop();
}

void AnalyzerDisplayWorker::start() {
    lastTickTimeMs = juce::Time::getMillisecondCounterHiRes();
    startThread();
}

void AnalyzerDisplayWorker::stop() {
    signalThreadShouldExit();
    wakeEvent.signal();
    stopThread(Display::Constants::workerStopTimeoutMs);
}

void AnalyzerDisplayWorker::setControlState(const AnalyzerDisplayControlState &newControlState) {
    {
        const juce::ScopedLock lock(controlLock);
        if (controlState == newControlState && !forceRefresh)
            return;

        controlState = newControlState;
        forceRefresh = true;
    }

    wakeEvent.signal();
}

const AnalyzerDisplayFrame *AnalyzerDisplayWorker::readLatestFrame(bool &hasUpdate) {
    return frameBuffer.getForReader(hasUpdate);
}

void AnalyzerDisplayWorker::run() {
    AnalyzerDisplayTickScheduler tickScheduler;
    tickScheduler.start(juce::Time::getMillisecondCounterHiRes(), Display::Constants::frameIntervalMs);
    bool wasGloballyFrozen = false;

    while (!threadShouldExit()) {
        bool shouldComputeImmediately = false;
        AnalyzerDisplayControlState currentControlState;

        {
            const juce::ScopedLock lock(controlLock);
            shouldComputeImmediately = forceRefresh;
            currentControlState = controlState;
        }

        if (currentControlState.globalFrozen && !shouldComputeImmediately) {
            // A control-state change signals the event, so a frozen display needs no polling.
            wakeEvent.wait();
            continue;
        }

        if (!shouldComputeImmediately) {
            const auto nowMs = juce::Time::getMillisecondCounterHiRes();
            wakeEvent.wait(tickScheduler.getWaitMilliseconds(nowMs));
        }

        if (threadShouldExit())
            break;

        {
            const juce::ScopedLock lock(controlLock);
            shouldComputeImmediately = forceRefresh;
            currentControlState = controlState;
        }

        const auto currentTimeMs = juce::Time::getMillisecondCounterHiRes();

        if (currentControlState.globalFrozen) {
            lastTickTimeMs = currentTimeMs;
            wasGloballyFrozen = true;
            const juce::ScopedLock lock(controlLock);
            forceRefresh = false;
            continue;
        }

        if (wasGloballyFrozen) {
            lastTickTimeMs = currentTimeMs;
            wasGloballyFrozen = false;
        }

        auto dtSeconds = static_cast<float>((currentTimeMs - lastTickTimeMs) * 0.001);
        if (dtSeconds < 0.0f)
            dtSeconds = 0.0f;

        bool wasForceRefresh = false;
        if (computeFrame(dtSeconds, wasForceRefresh))
            lastTickTimeMs = currentTimeMs;

        const auto nextIntervalMs = useIdlePolling
                                        ? Display::Constants::idlePollIntervalMs
                                        : Display::Constants::frameIntervalMs;
        tickScheduler.scheduleNext(juce::Time::getMillisecondCounterHiRes(),
                                   nextIntervalMs,
                                   wasForceRefresh);
    }
}

bool AnalyzerDisplayWorker::computeFrame(const float dtSeconds, bool &wasForceRefresh) {
    AnalyzerDisplayControlState currentControlState;
    {
        const juce::ScopedLock lock(controlLock);
        currentControlState = controlState;
        wasForceRefresh = forceRefresh;
        forceRefresh = false;
    }

    const auto nextBandInfo = rawTraceSource.getBandInfo();
    const auto bandLayoutChanged = currentBandInfo != nextBandInfo;
    const auto slotConfigurationChanged = previousControlState.slotConfigurations != currentControlState.slotConfigurations;
    if (bandLayoutChanged || slotConfigurationChanged) {
        currentBandInfo = nextBandInfo;
        resetDisplayModels();
    }
    displayFrameModel.syncControlState(previousControlState, currentControlState);

    const auto tracesView = rawTraceSource.readPublishedTraces();
    const auto shouldSuppressPendingTraceUpdate = bandLayoutChanged || slotConfigurationChanged;
    const auto& tracesToUse = shouldSuppressPendingTraceUpdate ? emptyRawTraces() : tracesView.getTraces();
    const auto hasTraceUpdate = tracesView.hasUpdate && !shouldSuppressPendingTraceUpdate;
    const auto shouldProcessAnalyzer = rawTraceSource.shouldProcessAnalyzer();
    const auto meterSettled = meter.isSettledAtFloor(currentControlState.floorDb);
    const auto holdSettled = globalHoldModel.isSettledAtFloor(currentControlState.floorDb);
    const auto shouldTick = wasForceRefresh
                            || bandLayoutChanged
                            || slotConfigurationChanged
                            || hasTraceUpdate
                            || shouldProcessAnalyzer
                            || !meterSettled
                            || !holdSettled;

    useIdlePolling = !shouldProcessAnalyzer && meterSettled && holdSettled;

    if (!shouldTick)
        return false;

    meter.tick(currentBandInfo,
               tracesToUse,
               hasTraceUpdate,
               !shouldProcessAnalyzer && !hasTraceUpdate,
               tracesView.hopDurationSeconds,
               currentControlState.meterSettings,
               currentControlState.floorDb,
               dtSeconds);

    AnalyzerContributingPeakSummary peakSummary;
    auto &nextFrame = frameBuffer.getForWriter();
    displayFrameModel.build(meter.getMeterData(),
                            currentBandInfo,
                            currentControlState,
                            currentControlState.floorDb,
                            nextFrame,
                            peakSummary);
    globalHoldModel.tick(peakSummary,
                         currentControlState.meterSettings,
                         currentControlState.floorDb,
                         dtSeconds);
    nextFrame.globalHoldFrame = globalHoldModel.getFrame();
    nextFrame.revision = nextRevision++;
    frameBuffer.publish();
    previousControlState = currentControlState;
    listener.analyzerDisplayFramePublished();
    return true;
}

void AnalyzerDisplayWorker::resetDisplayModels() {
    meter.reset();
    displayFrameModel.reset();
    globalHoldModel.reset();
}
