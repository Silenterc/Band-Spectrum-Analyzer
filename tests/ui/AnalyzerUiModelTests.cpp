#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "display/analyzer/data/AnalyzerContributingPeakSummary.h"
#include "display/analyzer/data/AnalyzerDisplayControlState.h"
#include "display/analyzer/data/AnalyzerDisplayFrame.h"
#include "display/analyzer/logic/AnalyzerDisplayFrameModel.h"
#include "display/analyzer/logic/AnalyzerGlobalHoldModel.h"
#include "ui/theme/UiTheme.h"
#include "ui/analyzer/plot/data/AnalyzerViewState.h"
#include "ui/analyzer/plot/logic/AnalyzerGeometry.h"
#include "ui/analyzer/plot/logic/AnalyzerHoverModel.h"
#include "ui/analyzer/plot/logic/AnalyzerRenderBatchBuilder.h"
#include "ui/analyzer/plot/logic/AnalyzerMeterTuning.h"
#include "ui/analyzer/plot/logic/AnalyzerUiSelectors.h"
#include "ui/analyzer/state/AnalyzerUiSnapshot.h"
#include "ui/analyzer/plot/logic/AnalyzerViewModel.h"
#include "ui/analyzer/rack/model/SignalRackModel.h"
#include "ui/analyzer/rack/model/SignalSlotOptions.h"

namespace {
    std::shared_ptr<const std::vector<Analyzer::BandInfo>> makeBandInfo(
        std::initializer_list<Analyzer::BandInfo> bands = {{.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f}}) {
        return std::make_shared<const std::vector<Analyzer::BandInfo>>(bands);
    }

    Analyzer::MeterData makeMeterData(const std::initializer_list<std::pair<Analyzer::TraceKind, float>> &peaksDb) {
        Analyzer::MeterData meterData;
        meterData.bandInfo = makeBandInfo();

        for (const auto &[kind, peakDb]: peaksDb) {
            Analyzer::MeterTrace trace;
            trace.kind = kind;
            trace.frame.rmsDb.push_back(peakDb);
            trace.frame.peakDb.push_back(peakDb);
            meterData.traces.push_back(std::move(trace));
        }

        return meterData;
    }

    Analyzer::MeterData makeZoomMeterData() {
        Analyzer::MeterData meterData;
        meterData.bandInfo = makeBandInfo({
            {.lowHz = 20.0f, .centerHz = 28.284271f, .highHz = 40.0f},
            {.lowHz = 40.0f, .centerHz = 56.568542f, .highHz = 80.0f},
            {.lowHz = 80.0f, .centerHz = 113.137085f, .highHz = 160.0f},
            {.lowHz = 160.0f, .centerHz = 226.27417f, .highHz = 320.0f}
        });

        Analyzer::MeterTrace trace;
        trace.kind = Analyzer::TraceKind::slot1;
        trace.frame.rmsDb = {-24.0f, -18.0f, -12.0f, -6.0f};
        trace.frame.peakDb = {-22.0f, -16.0f, -10.0f, -4.0f};
        meterData.traces.push_back(std::move(trace));
        return meterData;
    }

    Analyzer::MeterData makeMultiBandMeterData() {
        Analyzer::MeterData meterData;
        meterData.bandInfo = makeBandInfo({
            {.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f},
            {.lowHz = 125.0f, .centerHz = 160.0f, .highHz = 200.0f}
        });

        Analyzer::MeterTrace trace1;
        trace1.kind = Analyzer::TraceKind::slot1;
        trace1.frame.rmsDb = {-24.0f, -18.0f};
        trace1.frame.peakDb = {-22.0f, -16.0f};
        meterData.traces.push_back(std::move(trace1));

        Analyzer::MeterTrace trace2;
        trace2.kind = Analyzer::TraceKind::slot2;
        trace2.frame.rmsDb = {-12.0f, -9.0f};
        trace2.frame.peakDb = {-10.0f, -6.0f};
        meterData.traces.push_back(std::move(trace2));

        return meterData;
    }

    Shared::SignalSlotOrder makeDefaultSignalSlotOrder() {
        return {0, 1, 2, 3};
    }

    std::array<Ui::SignalSlotState, Shared::maxSignalSlots> makeVisibleSignalSlots() {
        std::array<Ui::SignalSlotState, Shared::maxSignalSlots> signalSlots{};

        for (size_t slotIndex = 0; slotIndex < signalSlots.size(); ++slotIndex) {
            signalSlots[slotIndex].configuration.enabled = true;
            signalSlots[slotIndex].visible = true;
            signalSlots[slotIndex].colourIndex = static_cast<int>(slotIndex);
        }

        return signalSlots;
    }

    AnalyzerDisplayControlState makeControlState(
        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
        const float floorDb) {
        AnalyzerDisplayControlState controlState;
        controlState.floorDb = floorDb;
        for (size_t slotIndex = 0; slotIndex < signalSlots.size(); ++slotIndex) {
            const auto &slot = signalSlots[slotIndex];
            controlState.slotFrozen[slotIndex] = slot.configuration.enabled && slot.frozen;
            controlState.slotContributing[slotIndex] = slot.configuration.enabled && slot.visible;
        }

        return controlState;
    }

    AnalyzerDisplayFrame buildDisplayFrame(const Analyzer::MeterData &meterData,
                                           const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                           const float floorDb,
                                           AnalyzerDisplayFrameModel &frameModel) {
        AnalyzerDisplayFrame frame;
        AnalyzerContributingPeakSummary peakSummary;
        const auto controlState = makeControlState(signalSlots, floorDb);
        frameModel.build(meterData, meterData.bandInfo, controlState, floorDb, frame, peakSummary);
        return frame;
    }

    AnalyzerContributingPeakSummary makeContributingPeakSummary(
        const Analyzer::MeterData &meterData,
        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
        const float floorDb) {
        AnalyzerDisplayFrameModel frameModel;
        AnalyzerDisplayFrame frame;
        AnalyzerContributingPeakSummary peakSummary;
        frameModel.build(meterData, meterData.bandInfo, makeControlState(signalSlots, floorDb), floorDb, frame, peakSummary);
        return peakSummary;
    }
}

TEST_CASE("AnalyzerUiSnapshot equality includes grid and freeze state", "[ui][snapshot]") {
    Ui::AnalyzerUiSnapshot lhs;
    Ui::AnalyzerUiSnapshot rhs;

    REQUIRE(lhs == rhs);

    rhs.gridMaxDb = 12.0f;
    REQUIRE(lhs != rhs);

    rhs = lhs;
    rhs.frozen = true;
    REQUIRE(lhs != rhs);
}

TEST_CASE("Visible trace kinds are derived from enabled and visible slots", "[ui][selectors]") {
    Ui::AnalyzerUiSnapshot snapshot;
    snapshot.signalSlots[0].configuration.enabled = true;
    snapshot.signalSlots[0].visible = true;
    snapshot.signalSlots[1].configuration.enabled = true;
    snapshot.signalSlots[1].visible = false;

    const auto visibleTraceKinds = Ui::collectVisibleTraceKinds(snapshot);
    REQUIRE(visibleTraceKinds.size() == 1);
    REQUIRE(visibleTraceKinds.front() == Analyzer::traceKindForSlot(0));
    REQUIRE(Ui::isTraceVisible(Analyzer::traceKindForSlot(0), snapshot.signalSlots));
    REQUIRE_FALSE(Ui::isTraceVisible(Analyzer::traceKindForSlot(1), snapshot.signalSlots));
}

TEST_CASE("Signal slot metadata drives labels and visible option counts", "[ui][metadata]") {
    REQUIRE(std::string(Ui::getSignalSourceLabel(Analyzer::SignalSource::main)) == "Main");
    REQUIRE(std::string(Ui::getSignalModeLabel(Analyzer::SignalMode::stereo)) == "Stereo");
    REQUIRE(Ui::getVisibleSignalSlotOptionCount(Analyzer::SignalSource::main, false) == 3);
    REQUIRE(Ui::getVisibleSignalSlotOptionCount(Analyzer::SignalSource::sidechain, false) == 0);
    REQUIRE(Ui::getVisibleSignalSlotOptionCount(Analyzer::SignalSource::sidechain, true) == 3);
}

TEST_CASE("Default signal configuration comes from centralized option metadata", "[ui][metadata]") {
    std::array<Ui::SignalSlotState, Shared::maxSignalSlots> signalSlots{};
    signalSlots[0].configuration.enabled = true;
    signalSlots[0].configuration.source = Analyzer::SignalSource::main;
    signalSlots[0].configuration.mode = Analyzer::SignalMode::mid;

    const auto configuration = Ui::chooseDefaultSignalConfiguration(signalSlots, false);
    REQUIRE(configuration.enabled);
    REQUIRE(configuration.source == Analyzer::SignalSource::main);
    REQUIRE(configuration.mode == Analyzer::SignalMode::side);
}

TEST_CASE("Hover readout matches the top-of-cursor visual reference", "[ui][hover]") {
    const auto theme = Ui::makeTheme();
    const AnalyzerGeometry geometry(theme);
    const FrequencyFormatter formatter;
    const MusicTheory musicTheory;
    const AnalyzerHoverModel hoverModel(geometry, formatter, musicTheory);

    const juce::Rectangle<float> localBounds(0.0f, 0.0f, 1000.0f, 600.0f);
    const auto plotBounds = geometry.getPlotBounds(localBounds);
    const std::vector<AnalyzerVisibleBandLayout> visibleBands{
        {.sourceBandIndex = 0,
         .hitBounds = geometry.getBandHitBounds(80.0f, 125.0f, 20.0f, 20000.0f, plotBounds),
         .drawBounds = geometry.getBandHitBounds(80.0f, 125.0f, 20.0f, 20000.0f, plotBounds)}
    };

    const auto hoverY = geometry.yForDb(-35.0f, -50.0f, 3.0f, plotBounds);
    const juce::Point<float> hoverPosition(visibleBands.front().hitBounds.getCentreX(), hoverY + 4.0f);
    const auto hoverInfo = hoverModel.build(localBounds, plotBounds, visibleBands,
                                            -50.0f, 3.0f, 20.0f, 20000.0f, hoverPosition);

    REQUIRE(hoverInfo.has_value());
    REQUIRE(hoverInfo->lineCount >= 1);
    REQUIRE(hoverInfo->lines[0] == "Volume: -35.0 dB");
}

TEST_CASE("Analyzer view model keeps full-range bar layout unchanged", "[ui][view-model]") {
    const auto theme = Ui::makeTheme();
    AnalyzerViewModel viewModel(theme);
    const auto meterData = makeZoomMeterData();
    AnalyzerViewState viewState;

    viewModel.updateStaticLayout(*meterData.bandInfo, viewState, -50.0f, 3.0f, 6.0f, {0.0f, 0.0f, 1000.0f, 600.0f});

    const auto &visibleBands = viewModel.getVisibleBands();
    REQUIRE(visibleBands.size() == meterData.bandInfo->size());
    REQUIRE(visibleBands.front().sourceBandIndex == 0);
    REQUIRE(visibleBands.back().sourceBandIndex == meterData.bandInfo->size() - 1);

    const auto &frequencyMarkers = viewModel.getFrequencyMarkers();
    REQUIRE_FALSE(frequencyMarkers.empty());
    REQUIRE(frequencyMarkers.front().label == "20");
    REQUIRE(frequencyMarkers.back().label.contains("20"));
    REQUIRE(frequencyMarkers.back().label.contains("k"));
}

TEST_CASE("Analyzer view model reflows visible bands and hover with custom frequency range", "[ui][view-model][hover]") {
    const auto theme = Ui::makeTheme();
    AnalyzerViewModel viewModel(theme);
    const auto meterData = makeZoomMeterData();
    AnalyzerViewState viewState;
    viewState.useCustomFrequencyRange = true;
    viewState.visibleMinFrequencyHz = 45.0f;
    viewState.visibleMaxFrequencyHz = 210.0f;

    viewModel.updateStaticLayout(*meterData.bandInfo, viewState, -50.0f, 3.0f, 6.0f, {0.0f, 0.0f, 1000.0f, 600.0f});

    const auto &visibleBands = viewModel.getVisibleBands();
    REQUIRE(visibleBands.size() == 3);
    REQUIRE(visibleBands[0].sourceBandIndex == 1);
    REQUIRE(visibleBands[1].sourceBandIndex == 2);
    REQUIRE(visibleBands[2].sourceBandIndex == 3);

    const auto plotBounds = viewModel.getPlotBounds();
    REQUIRE(visibleBands[0].drawBounds.getX() == Catch::Approx(plotBounds.getX()));
    REQUIRE(visibleBands[2].drawBounds.getRight() == Catch::Approx(plotBounds.getRight()));

    const juce::Point<float> hoverPosition(visibleBands[1].drawBounds.getCentreX(), plotBounds.getCentreY());
    viewModel.updateHover(-50.0f, 3.0f, {0.0f, 0.0f, 1000.0f, 600.0f}, hoverPosition);

    REQUIRE(viewModel.getHoverInfo().has_value());
    REQUIRE(viewModel.getHoverInfo()->bandIndex == 1);
    REQUIRE(visibleBands[viewModel.getHoverInfo()->bandIndex].sourceBandIndex == 2);

    const auto &frequencyMarkers = viewModel.getFrequencyMarkers();
    REQUIRE_FALSE(frequencyMarkers.empty());
    REQUIRE(frequencyMarkers.front().label == "50");
    REQUIRE(frequencyMarkers.back().label == "200");
}

TEST_CASE("Global hold takes the maximum of currently visible traces", "[ui][global-hold]") {
    AnalyzerGlobalHoldModel holdModel;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showHold = true;
    meterSettings.holdMs = 200.0f;

    auto signalSlots = makeVisibleSignalSlots();
    signalSlots[1].visible = false;

    const auto meterData = makeMeterData({
        {Analyzer::TraceKind::slot1, -12.0f},
        {Analyzer::TraceKind::slot2, -3.0f}
    });

    holdModel.tick(makeContributingPeakSummary(meterData, signalSlots, -50.0f), meterSettings, -50.0f, 0.016f);

    REQUIRE(holdModel.getFrame().has_value());
    REQUIRE(holdModel.getFrame()->holdDb.size() == 1);
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(-12.0f));
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
}

TEST_CASE("Global hold latches owner until a stronger visible peak replaces it", "[ui][global-hold]") {
    AnalyzerGlobalHoldModel holdModel;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showHold = true;
    meterSettings.holdMs = 200.0f;

    const auto signalSlots = makeVisibleSignalSlots();

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot1, -6.0f}}),
                                          signalSlots,
                                          -50.0f),
                   meterSettings,
                   -50.0f,
                   0.016f);
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(-6.0f));

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot2, -12.0f}}),
                                          signalSlots,
                                          -50.0f),
                   meterSettings,
                   -50.0f,
                   0.050f);
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(-6.0f));

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot2, -2.0f}}),
                                          signalSlots,
                                          -50.0f),
                   meterSettings,
                   -50.0f,
                   0.016f);
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot2);
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(-2.0f));
}

TEST_CASE("Global hold decays after hold time elapses", "[ui][global-hold]") {
    AnalyzerGlobalHoldModel holdModel;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showHold = true;
    meterSettings.holdMs = 100.0f;

    const auto signalSlots = makeVisibleSignalSlots();
    const auto floorDb = -50.0f;

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot1, 0.0f}}),
                                          signalSlots,
                                          floorDb),
                   meterSettings,
                   floorDb,
                   0.016f);

    holdModel.tick(makeContributingPeakSummary(makeMeterData({}), signalSlots, floorDb),
                   meterSettings,
                   floorDb,
                   0.100f);
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(0.0f));

    holdModel.tick(makeContributingPeakSummary(makeMeterData({}), signalSlots, floorDb),
                   meterSettings,
                   floorDb,
                   0.250f);
    REQUIRE(holdModel.getFrame()->holdDb[0]
            == Catch::Approx(-0.250f * Ui::analyzerMeterTuning.holdDecayDbPerSecond));
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
}

TEST_CASE("Global hold snaps exactly to the floor once it settles", "[ui][global-hold]") {
    AnalyzerGlobalHoldModel holdModel;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showHold = true;
    meterSettings.holdMs = 0.0f;

    const auto signalSlots = makeVisibleSignalSlots();
    const auto floorDb = -50.0f;

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot1, -49.95f}}),
                                          signalSlots,
                                          floorDb),
                   meterSettings,
                   floorDb,
                   0.016f);
    REQUIRE(holdModel.getFrame().has_value());
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(-49.95f));

    holdModel.tick(makeContributingPeakSummary(makeMeterData({}), signalSlots, floorDb),
                   meterSettings,
                   floorDb,
                   0.250f);

    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(floorDb));
    REQUIRE_FALSE(holdModel.getFrame()->ownerKinds[0].has_value());
    REQUIRE(holdModel.isSettledAtFloor(floorDb));
}

TEST_CASE("Global hold settles onto the currently rendered lower peak once decay reaches it",
          "[ui][global-hold]") {
    AnalyzerGlobalHoldModel holdModel;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showHold = true;
    meterSettings.holdMs = 0.0f;

    const auto signalSlots = makeVisibleSignalSlots();
    const auto floorDb = -50.0f;

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot1, 0.0f}}),
                                          signalSlots,
                                          floorDb),
                   meterSettings,
                   floorDb,
                   0.016f);

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot1, -12.0f}}),
                                          signalSlots,
                                          floorDb),
                   meterSettings,
                   floorDb,
                   1.900f);

    REQUIRE(holdModel.getFrame().has_value());
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(-12.0f));
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
}

TEST_CASE("Global hold refreshes hold time for lower peaks within reset tolerance",
          "[ui][global-hold]") {
    AnalyzerGlobalHoldModel holdModel;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showHold = true;
    meterSettings.holdMs = 200.0f;

    const auto signalSlots = makeVisibleSignalSlots();
    const auto floorDb = -50.0f;

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot1, 0.0f}}),
                                          signalSlots,
                                          floorDb),
                   meterSettings,
                   floorDb,
                   0.016f);

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot1, -0.5f}}),
                                          signalSlots,
                                          floorDb),
                   meterSettings,
                   floorDb,
                   0.150f);

    holdModel.tick(makeContributingPeakSummary(makeMeterData({{Analyzer::TraceKind::slot1, -0.5f}}),
                                          signalSlots,
                                          floorDb),
                   meterSettings,
                   floorDb,
                   0.150f);

    REQUIRE(holdModel.getFrame().has_value());
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(0.0f));
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
}

TEST_CASE("Display frame model excludes hidden traces from contributing peak summary",
          "[ui][display-frame]") {
    AnalyzerDisplayFrameModel frameModel;
    auto signalSlots = makeVisibleSignalSlots();
    signalSlots[1].visible = false;

    AnalyzerDisplayFrame frame;
    AnalyzerContributingPeakSummary peakSummary;
    const auto meterData = makeMultiBandMeterData();
    frameModel.build(meterData, meterData.bandInfo, makeControlState(signalSlots, -50.0f), -50.0f, frame, peakSummary);

    REQUIRE(frame.slotFrames[0].active);
    REQUIRE(frame.slotFrames[1].active);
    REQUIRE(peakSummary.peakDb.size() == 2);
    REQUIRE(peakSummary.peakDb[0] == Catch::Approx(-22.0f));
    REQUIRE(peakSummary.peakDb[1] == Catch::Approx(-16.0f));
    REQUIRE(peakSummary.ownerKinds[0] == Analyzer::TraceKind::slot1);
    REQUIRE(peakSummary.ownerKinds[1] == Analyzer::TraceKind::slot1);
}

TEST_CASE("Display frame model reuses frozen trace while live data continues updating",
          "[ui][display-frame]") {
    AnalyzerDisplayFrameModel frameModel;
    auto previousControlState = makeControlState(makeVisibleSignalSlots(), -50.0f);
    auto frozenSignalSlots = makeVisibleSignalSlots();
    frozenSignalSlots[0].frozen = true;
    auto currentControlState = makeControlState(frozenSignalSlots, -50.0f);

    AnalyzerDisplayFrame firstFrame;
    AnalyzerContributingPeakSummary firstSummary;
    const auto initialMeterData = makeMeterData({{Analyzer::TraceKind::slot1, -12.0f}});
    frameModel.build(initialMeterData,
                     initialMeterData.bandInfo,
                     previousControlState,
                     -50.0f,
                     firstFrame,
                     firstSummary);
    frameModel.syncControlState(previousControlState, currentControlState);

    AnalyzerDisplayFrame frozenFrame;
    AnalyzerContributingPeakSummary frozenSummary;
    const auto liveMeterData = makeMeterData({{Analyzer::TraceKind::slot1, -6.0f}});
    frameModel.build(liveMeterData,
                     liveMeterData.bandInfo,
                     currentControlState,
                     -50.0f,
                     frozenFrame,
                     frozenSummary);

    REQUIRE(frozenFrame.slotFrames[0].active);
    REQUIRE(frozenFrame.slotFrames[0].frame.peakDb[0] == Catch::Approx(-12.0f));
    REQUIRE(frozenSummary.peakDb[0] == Catch::Approx(-12.0f));
    REQUIRE(frozenSummary.ownerKinds[0] == Analyzer::TraceKind::slot1);
}

TEST_CASE("Display frame model clears frozen cache when slot is unfrozen",
          "[ui][display-frame]") {
    AnalyzerDisplayFrameModel frameModel;
    auto previousSignalSlots = makeVisibleSignalSlots();
    auto frozenSignalSlots = previousSignalSlots;
    frozenSignalSlots[0].frozen = true;
    auto frozenControlState = makeControlState(frozenSignalSlots, -50.0f);

    AnalyzerDisplayFrame initialFrame;
    AnalyzerContributingPeakSummary initialSummary;
    const auto initialMeterData = makeMeterData({{Analyzer::TraceKind::slot1, -12.0f}});
    frameModel.build(initialMeterData,
                     initialMeterData.bandInfo,
                     makeControlState(previousSignalSlots, -50.0f),
                     -50.0f,
                     initialFrame,
                     initialSummary);
    frameModel.syncControlState(makeControlState(previousSignalSlots, -50.0f), frozenControlState);
    frameModel.syncControlState(frozenControlState, makeControlState(previousSignalSlots, -50.0f));

    AnalyzerDisplayFrame resultFrame;
    AnalyzerContributingPeakSummary resultSummary;
    const auto liveMeterData = makeMeterData({{Analyzer::TraceKind::slot1, -6.0f}});
    frameModel.build(liveMeterData,
                     liveMeterData.bandInfo,
                     makeControlState(previousSignalSlots, -50.0f),
                     -50.0f,
                     resultFrame,
                     resultSummary);
    REQUIRE(resultFrame.slotFrames[0].frame.peakDb[0] == Catch::Approx(-6.0f));
}

TEST_CASE("Display frame model preserves a hidden slot's frozen snapshot until it becomes visible",
          "[ui][display-frame]") {
    AnalyzerDisplayFrameModel frameModel;
    auto previousSignalSlots = makeVisibleSignalSlots();
    auto hiddenFrozenSignalSlots = previousSignalSlots;
    hiddenFrozenSignalSlots[0].visible = false;
    hiddenFrozenSignalSlots[0].frozen = true;
    const auto initialControlState = makeControlState(previousSignalSlots, -50.0f);
    const auto hiddenControlState = makeControlState(hiddenFrozenSignalSlots, -50.0f);

    AnalyzerDisplayFrame initialFrame;
    AnalyzerContributingPeakSummary initialSummary;
    const auto initialMeterData = makeMeterData({{Analyzer::TraceKind::slot1, -12.0f}});
    frameModel.build(initialMeterData,
                     initialMeterData.bandInfo,
                     initialControlState,
                     -50.0f,
                     initialFrame,
                     initialSummary);
    frameModel.syncControlState(initialControlState, hiddenControlState);

    AnalyzerDisplayFrame hiddenFrame;
    AnalyzerContributingPeakSummary hiddenSummary;
    const auto hiddenMeterData = makeMeterData({{Analyzer::TraceKind::slot1, -6.0f}});
    frameModel.build(hiddenMeterData,
                     hiddenMeterData.bandInfo,
                     hiddenControlState,
                     -50.0f,
                     hiddenFrame,
                     hiddenSummary);
    REQUIRE(hiddenFrame.slotFrames[0].active);
    REQUIRE(hiddenSummary.ownerKinds[0] == std::nullopt);

    auto visibleFrozenSignalSlots = hiddenFrozenSignalSlots;
    visibleFrozenSignalSlots[0].visible = true;
    AnalyzerDisplayFrame visibleFrame;
    AnalyzerContributingPeakSummary visibleSummary;
    const auto visibleControlState = makeControlState(visibleFrozenSignalSlots, -50.0f);
    const auto visibleMeterData = makeMeterData({{Analyzer::TraceKind::slot1, -6.0f}});
    frameModel.build(visibleMeterData,
                     visibleMeterData.bandInfo,
                     visibleControlState,
                     -50.0f,
                     visibleFrame,
                     visibleSummary);

    REQUIRE(visibleFrame.slotFrames[0].active);
    REQUIRE(visibleFrame.slotFrames[0].frame.peakDb[0] == Catch::Approx(-12.0f));
    REQUIRE(visibleSummary.peakDb[0] == Catch::Approx(-12.0f));
    REQUIRE(visibleSummary.ownerKinds[0] == Analyzer::TraceKind::slot1);
}

TEST_CASE("Render batching preserves visible analyzer bar and hold rectangles", "[ui][render-batching]") {
    const auto theme = Ui::makeTheme();
    AnalyzerViewModel viewModel(theme);
    AnalyzerViewState viewState;
    const auto meterData = makeMultiBandMeterData();
    const auto signalSlots = makeVisibleSignalSlots();
    AnalyzerDisplayFrameModel frameModel;
    auto displayFrame = buildDisplayFrame(meterData, signalSlots, -50.0f, frameModel);
    displayFrame.globalHoldFrame = AnalyzerGlobalHoldFrame{
        .holdDb = {-22.0f, -6.0f},
        .ownerKinds = {Analyzer::TraceKind::slot1, Analyzer::TraceKind::slot2}
    };

    viewModel.updateStaticLayout(*meterData.bandInfo, viewState, -50.0f, 3.0f, 6.0f, {0.0f, 0.0f, 1000.0f, 600.0f});

    AnalyzerRenderBatchBuilder batchBuilder(theme);
    const auto clipBounds = juce::Rectangle<float>(0.0f, 0.0f, 1000.0f, 600.0f);
    const auto plotBounds = viewModel.getPlotBounds();

    batchBuilder.buildTraceBatches(&displayFrame,
                                   viewModel.getVisibleBands(),
                                   signalSlots,
                                   makeDefaultSignalSlotOrder(),
                                   Analyzer::MeterSettings{.showRms = true, .showPeak = true},
                                   viewModel.getGridMinDb(),
                                   3.0f,
                                   plotBounds,
                                   clipBounds);
    int totalBarRects = 0;
    for (const auto &batch: batchBuilder.getBatches())
        totalBarRects += batch.rectangles.getNumRectangles();
    REQUIRE(totalBarRects == 8);

    batchBuilder.buildGlobalHoldBatches(&displayFrame,
                                        viewModel.getVisibleBands(),
                                        signalSlots,
                                        Analyzer::MeterSettings{.showHold = true},
                                        viewModel.getGridMinDb(),
                                        3.0f,
                                        plotBounds,
                                        clipBounds);
    int totalHoldRects = 0;
    for (const auto &batch: batchBuilder.getBatches())
        totalHoldRects += batch.rectangles.getNumRectangles();
    REQUIRE(totalHoldRects == 2);

    batchBuilder.buildHoveredBarBatches(&displayFrame,
                                        viewModel.getVisibleBands(),
                                        signalSlots,
                                        makeDefaultSignalSlotOrder(),
                                        Analyzer::MeterSettings{.showRms = true, .showPeak = true},
                                        0,
                                        viewModel.getGridMinDb(),
                                        3.0f,
                                        plotBounds,
                                        clipBounds);
    int totalHoveredRects = 0;
    for (const auto &batch: batchBuilder.getBatches())
        totalHoveredRects += batch.rectangles.getNumRectangles();
    REQUIRE(totalHoveredRects == 4);
}
