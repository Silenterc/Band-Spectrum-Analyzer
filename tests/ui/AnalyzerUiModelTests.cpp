#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../../src/ui/UiTheme.h"
#include "../../src/ui/analyzer/helpers/AnalyzerGeometry.h"
#include "../../src/ui/analyzer/helpers/AnalyzerHoverModel.h"
#include "../../src/ui/analyzer/model/AnalyzerGlobalHoldModel.h"
#include "../../src/ui/analyzer/model/AnalyzerMeterTuning.h"
#include "../../src/ui/analyzer/model/AnalyzerUiSelectors.h"
#include "../../src/ui/analyzer/model/AnalyzerUiSnapshot.h"
#include "../../src/ui/analyzer/model/SignalRackModel.h"
#include "../../src/ui/analyzer/model/SignalSlotOptions.h"

namespace {
    Analyzer::RenderData makeRenderData(const std::initializer_list<std::pair<Analyzer::TraceKind, float>> &peaksDb) {
        Analyzer::RenderData renderData;
        renderData.bandInfo.push_back({.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f});

        for (const auto &[kind, peakDb]: peaksDb) {
            Analyzer::RenderTrace trace;
            trace.kind = kind;
            trace.frame.rmsDb.push_back(peakDb);
            trace.frame.peakDb.push_back(peakDb);
            renderData.traces.push_back(std::move(trace));
        }

        return renderData;
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
    const std::vector<Analyzer::BandInfo> bandInfo{
        {.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f}
    };

    const auto hoverY = geometry.yForDb(-35.0f, -50.0f, 3.0f, plotBounds);
    const juce::Point<float> hoverPosition(plotBounds.getCentreX(), hoverY + 4.0f);
    const auto hoverInfo = hoverModel.build(localBounds, plotBounds, bandInfo,
                                            -50.0f, 3.0f, 20.0f, 20000.0f, hoverPosition);

    REQUIRE(hoverInfo.has_value());
    REQUIRE(hoverInfo->lineCount >= 1);
    REQUIRE(hoverInfo->lines[0] == "Volume: -35.0 dB");
}

TEST_CASE("Global hold takes the maximum of currently visible traces", "[ui][global-hold]") {
    AnalyzerGlobalHoldModel holdModel;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showHold = true;
    meterSettings.holdMs = 200.0f;

    auto signalSlots = makeVisibleSignalSlots();
    signalSlots[1].visible = false;

    const auto renderData = makeRenderData({
        {Analyzer::TraceKind::slot1, -12.0f},
        {Analyzer::TraceKind::slot2, -3.0f}
    });

    holdModel.tick(renderData, signalSlots, meterSettings, -50.0f, 0.016f);

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

    holdModel.tick(makeRenderData({{Analyzer::TraceKind::slot1, -6.0f}}),
                   signalSlots,
                   meterSettings,
                   -50.0f,
                   0.016f);
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(-6.0f));

    holdModel.tick(makeRenderData({{Analyzer::TraceKind::slot2, -12.0f}}),
                   signalSlots,
                   meterSettings,
                   -50.0f,
                   0.050f);
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(-6.0f));

    holdModel.tick(makeRenderData({{Analyzer::TraceKind::slot2, -2.0f}}),
                   signalSlots,
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

    holdModel.tick(makeRenderData({{Analyzer::TraceKind::slot1, 0.0f}}),
                   signalSlots,
                   meterSettings,
                   floorDb,
                   0.016f);

    holdModel.tick(makeRenderData({}),
                   signalSlots,
                   meterSettings,
                   floorDb,
                   0.100f);
    REQUIRE(holdModel.getFrame()->holdDb[0] == Catch::Approx(0.0f));

    holdModel.tick(makeRenderData({}),
                   signalSlots,
                   meterSettings,
                   floorDb,
                   0.250f);
    REQUIRE(holdModel.getFrame()->holdDb[0]
            == Catch::Approx(-0.250f * Ui::analyzerMeterTuning.holdDecayDbPerSecond));
    REQUIRE(holdModel.getFrame()->ownerKinds[0] == Analyzer::TraceKind::slot1);
}
