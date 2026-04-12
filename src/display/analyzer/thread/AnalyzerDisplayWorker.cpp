#include "display/analyzer/thread/AnalyzerDisplayWorker.h"

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
    wakeEvent.signal();
}

void AnalyzerDisplayWorker::stop() {
    signalThreadShouldExit();
    wakeEvent.signal();
    stopThread(2000);
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
    while (!threadShouldExit()) {
        bool shouldCompute = false;

        {
            const juce::ScopedLock lock(controlLock);
            shouldCompute = forceRefresh;
        }

        const auto waitMs = shouldCompute ? 0 : (useIdlePolling
                                                     ? Ui::AnalyzerConstants::idleMeterPollIntervalMs
                                                     : Ui::AnalyzerConstants::meterPollIntervalMs);

        if (!shouldCompute)
            wakeEvent.wait(waitMs);

        if (threadShouldExit())
            break;

        AnalyzerDisplayControlState currentControlState;
        {
            const juce::ScopedLock lock(controlLock);
            currentControlState = controlState;
        }

        double currentTimeMs = juce::Time::getMillisecondCounterHiRes();
        auto dtSeconds = static_cast<float>((currentTimeMs - lastTickTimeMs) * 0.001);
        if (dtSeconds < 0.0f)
            dtSeconds = 0.0f;

        if (currentControlState.globalFrozen) {
            // Global freeze pins the last published frame and stops meter and hold time from advancing.
            lastTickTimeMs = currentTimeMs;
            useIdlePolling = false;
            const juce::ScopedLock lock(controlLock);
            forceRefresh = false;
            continue;
        }

        computeFrame(dtSeconds);
        lastTickTimeMs = currentTimeMs;
    }
}

void AnalyzerDisplayWorker::computeFrame(const float dtSeconds) {
    AnalyzerDisplayControlState currentControlState;
    bool shouldForceRefresh = false;
    {
        const juce::ScopedLock lock(controlLock);
        currentControlState = controlState;
        shouldForceRefresh = forceRefresh;
        forceRefresh = false;
    }

    const auto nextBandInfo = rawTraceSource.getBandInfo();
    const auto bandLayoutChanged = currentBandInfo != nextBandInfo;
    if (bandLayoutChanged) {
        currentBandInfo = nextBandInfo;
        resetDisplayModels();
    }
    displayFrameModel.syncControlState(previousControlState, currentControlState);

    const auto tracesView = rawTraceSource.readPublishedTraces();
    const auto hasRecentSignal = rawTraceSource.hasRecentSignal();
    const auto shouldTick = shouldForceRefresh
                            || bandLayoutChanged
                            || tracesView.hasUpdate
                            || hasRecentSignal
                            || !meter.isSettledAtFloor(currentControlState.floorDb)
                            || !globalHoldModel.isSettledAtFloor(currentControlState.floorDb);

    useIdlePolling = !hasRecentSignal
                     && meter.isSettledAtFloor(currentControlState.floorDb)
                     && globalHoldModel.isSettledAtFloor(currentControlState.floorDb);

    if (!shouldTick && !bandLayoutChanged)
        return;

    meter.tick(currentBandInfo,
               tracesView.getTraces(),
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
}

void AnalyzerDisplayWorker::resetDisplayModels() {
    meter.reset();
    displayFrameModel.reset();
    globalHoldModel.reset();
}
