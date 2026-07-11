#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "ui/settings/model/SettingsRangeModel.h"

TEST_CASE("Grid min allowed range stays a span below grid max", "[ui][settings][range]") {
    Ui::AnalyzerUiSnapshot snapshot;
    snapshot.gridMaxDb = -10.0f;

    const auto allowed = Ui::SettingsRangeModel::gridMinDbAllowedRange(snapshot);
    REQUIRE(allowed.minimum == Catch::Approx(Defaults::gridMinDbMin));
    REQUIRE(allowed.maximum == Catch::Approx(-10.0f - Defaults::gridMinSpanDb));
}

TEST_CASE("Grid min allowed range keeps the schema cap when grid max is high", "[ui][settings][range]") {
    Ui::AnalyzerUiSnapshot snapshot;
    snapshot.gridMaxDb = Defaults::gridMaxDbMax;

    const auto allowed = Ui::SettingsRangeModel::gridMinDbAllowedRange(snapshot);
    REQUIRE(allowed.maximum == Catch::Approx(Defaults::gridMinDbMax));
}

TEST_CASE("Grid max allowed range stays a span above grid min", "[ui][settings][range]") {
    Ui::AnalyzerUiSnapshot snapshot;
    snapshot.gridMinDb = -12.0f;

    const auto allowed = Ui::SettingsRangeModel::gridMaxDbAllowedRange(snapshot);
    REQUIRE(allowed.minimum == Catch::Approx(-12.0f + Defaults::gridMinSpanDb));
    REQUIRE(allowed.maximum == Catch::Approx(Defaults::gridMaxDbMax));
}

TEST_CASE("Visible min frequency allowed range stays an octave below max", "[ui][settings][range]") {
    Ui::AnalyzerUiSnapshot snapshot;
    snapshot.visibleMaxFrequencyHz = 20000.0f;

    const auto allowed = Ui::SettingsRangeModel::visibleMinFrequencyAllowedRange(snapshot);
    REQUIRE(allowed.maximum == Catch::Approx(20000.0f / Defaults::visibleFrequencyMinSpanRatio));
}

TEST_CASE("Visible frequency allowed ranges never invert on degenerate input", "[ui][settings][range]") {
    Ui::AnalyzerUiSnapshot snapshot;
    snapshot.visibleMaxFrequencyHz = Defaults::visibleMinFrequencyHzMin;
    snapshot.visibleMinFrequencyHz = Defaults::visibleMaxFrequencyHzMax;

    const auto minAllowed = Ui::SettingsRangeModel::visibleMinFrequencyAllowedRange(snapshot);
    const auto maxAllowed = Ui::SettingsRangeModel::visibleMaxFrequencyAllowedRange(snapshot);
    REQUIRE(minAllowed.minimum <= minAllowed.maximum);
    REQUIRE(maxAllowed.minimum <= maxAllowed.maximum);
}

TEST_CASE("Visible max frequency allowed range stays an octave above min", "[ui][settings][range]") {
    Ui::AnalyzerUiSnapshot snapshot;
    snapshot.visibleMinFrequencyHz = 10000.0f;

    const auto allowed = Ui::SettingsRangeModel::visibleMaxFrequencyAllowedRange(snapshot);
    REQUIRE(allowed.minimum == Catch::Approx(10000.0f * Defaults::visibleFrequencyMinSpanRatio));
}
