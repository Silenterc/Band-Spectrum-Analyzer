#include <atomic>
#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "display/analyzer/thread/AnalyzerDisplayWorker.h"

namespace {
    class TestRawTraceSource final : public AnalyzerRawTraceSource {
    public:
        TestRawTraceSource()
            : bandInfo(std::make_shared<const std::vector<Analyzer::BandInfo>>(
                  std::vector<Analyzer::BandInfo>{{20.0f, 40.0f, 80.0f}})),
              loudTraces{{Analyzer::TraceKind::slot1, {{1.0f, 1.0, 1}}}},
              silentTraces{{Analyzer::TraceKind::slot1, {{0.0f, 0.0, 1}}}} {
        }

        std::shared_ptr<const std::vector<Analyzer::BandInfo>> getBandInfo() const override {
            return bandInfo;
        }

        AnalyzerPublishedTracesView readPublishedTraces() const override {
            const auto useSilentTrace = silent.load(std::memory_order_acquire);
            return {
                .traces = useSilentTrace ? &silentTraces : &loudTraces,
                .hasUpdate = true,
                .hopDurationSeconds = 0.02f
            };
        }

        bool hasRecentSignal() const override {
            return true;
        }

        bool shouldProcessAnalyzer() const override {
            return true;
        }

        void publishSilence() {
            silent.store(true, std::memory_order_release);
        }

    private:
        std::shared_ptr<const std::vector<Analyzer::BandInfo>> bandInfo;
        std::vector<Analyzer::RawTrace> loudTraces;
        std::vector<Analyzer::RawTrace> silentTraces;
        std::atomic<bool> silent{false};
    };

    class FreezeOnFirstFrameListener final : public AnalyzerDisplayWorker::Listener {
    public:
        void analyzerDisplayFramePublished() override {
            publicationCount.fetch_add(1, std::memory_order_acq_rel);

            bool hasUpdate = false;
            const auto *frame = worker->readLatestFrame(hasUpdate);
            const auto hasActivePeak = frame != nullptr
                                       && frame->slotFrames[0].active
                                       && frame->slotFrames[0].frame.peakDb.size() == 1
                                       && frame->slotFrames[0].frame.peakDb[0] > -0.5f;
            if (!freezeStarted.load(std::memory_order_acquire) && hasActivePeak) {
                freezeStarted.store(true, std::memory_order_release);
                worker->setControlState(frozenControlState);
                firstFramePublished.signal();
                return;
            }

            if (freezeStarted.load(std::memory_order_acquire))
                resumedFramePublished.signal();
        }

        AnalyzerDisplayWorker *worker = nullptr;
        AnalyzerDisplayControlState frozenControlState;
        std::atomic<int> publicationCount{0};
        std::atomic<bool> freezeStarted{false};
        juce::WaitableEvent firstFramePublished;
        juce::WaitableEvent resumedFramePublished;
    };
}

TEST_CASE("Global freeze excludes frozen time from display decay") {
    TestRawTraceSource source;
    FreezeOnFirstFrameListener listener;
    AnalyzerDisplayWorker worker(source, listener);
    listener.worker = &worker;

    AnalyzerDisplayControlState activeControlState;
    activeControlState.floorDb = -100.0f;
    activeControlState.meterSettings.peakDecayDbPerSecond = 60.0f;
    listener.frozenControlState = activeControlState;
    listener.frozenControlState.globalFrozen = true;

    worker.setControlState(activeControlState);
    worker.start();

    REQUIRE(listener.firstFramePublished.wait(500));
    const auto publicationCountAtFreeze = listener.publicationCount.load(std::memory_order_acquire);
    juce::Thread::sleep(120);
    REQUIRE(listener.publicationCount.load(std::memory_order_acquire) == publicationCountAtFreeze);

    source.publishSilence();
    worker.setControlState(activeControlState);
    REQUIRE(listener.resumedFramePublished.wait(500));

    bool hasUpdate = false;
    const auto *frame = worker.readLatestFrame(hasUpdate);
    REQUIRE(frame != nullptr);
    REQUIRE(frame->slotFrames[0].active);
    REQUIRE(frame->slotFrames[0].frame.peakDb.size() == 1);
    REQUIRE(frame->slotFrames[0].frame.peakDb[0] > -0.5f);
}
