#include <catch2/catch_test_macros.hpp>

#include "display/analyzer/thread/AnalyzerDisplayTickScheduler.h"

TEST_CASE("Analyzer display deadlines stay anchored to the requested cadence") {
    AnalyzerDisplayTickScheduler scheduler;
    scheduler.start(100.0, 20);

    REQUIRE(scheduler.getWaitMilliseconds(100.0) == 20);

    scheduler.scheduleNext(120.6, 20, false);
    REQUIRE(scheduler.getWaitMilliseconds(120.6) == 20);

    scheduler.scheduleNext(140.6, 20, false);
    REQUIRE(scheduler.getWaitMilliseconds(140.6) == 20);
}

TEST_CASE("Analyzer display deadlines skip overruns instead of catching up") {
    AnalyzerDisplayTickScheduler scheduler;
    scheduler.start(100.0, 20);

    scheduler.scheduleNext(147.0, 20, false);
    REQUIRE(scheduler.getWaitMilliseconds(147.0) == 13);
}

TEST_CASE("Analyzer display deadlines reanchor for forced and cadence changes") {
    AnalyzerDisplayTickScheduler scheduler;
    scheduler.start(100.0, 20);

    scheduler.scheduleNext(110.0, 20, true);
    REQUIRE(scheduler.getWaitMilliseconds(110.0) == 20);

    scheduler.scheduleNext(130.0, 120, false);
    REQUIRE(scheduler.getWaitMilliseconds(130.0) == 120);
}

TEST_CASE("Analyzer display deadlines preserve fractional frame intervals") {
    AnalyzerDisplayTickScheduler scheduler;
    constexpr double sixtyFpsIntervalMs = 1000.0 / 60.0;
    scheduler.start(100.0, sixtyFpsIntervalMs);

    REQUIRE(scheduler.getWaitMilliseconds(100.0) == 17);

    scheduler.scheduleNext(100.0 + sixtyFpsIntervalMs, sixtyFpsIntervalMs, false);
    REQUIRE(scheduler.getWaitMilliseconds(100.0 + sixtyFpsIntervalMs) == 17);
}
