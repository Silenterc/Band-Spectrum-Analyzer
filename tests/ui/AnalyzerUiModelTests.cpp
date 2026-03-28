#include <catch2/catch_test_macros.hpp>

#include "../../src/ui/analyzer/model/AnalyzerUiSelectors.h"
#include "../../src/ui/analyzer/model/AnalyzerUiSnapshot.h"
#include "../../src/ui/analyzer/model/SignalRackModel.h"
#include "../../src/ui/analyzer/model/SignalSlotOptions.h"

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
