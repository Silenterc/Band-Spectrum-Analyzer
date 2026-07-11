#pragma once

#include <algorithm>
#include <cmath>

class AnalyzerDisplayTickScheduler final {
public:
    void start(const double nowMs, const double intervalMsToUse) {
        intervalMs = std::max(1.0, intervalMsToUse);
        nextDeadlineMs = nowMs + intervalMs;
    }

    [[nodiscard]] int getWaitMilliseconds(const double nowMs) const {
        return std::max(0, static_cast<int>(std::ceil(nextDeadlineMs - nowMs)));
    }

    void scheduleNext(const double nowMs, const double intervalMsToUse, const bool reanchor) {
        const auto nextIntervalMs = std::max(1.0, intervalMsToUse);
        const auto intervalChanged = std::abs(nextIntervalMs - intervalMs) > 1.0e-9;
        if (reanchor || intervalChanged) {
            start(nowMs, nextIntervalMs);
            return;
        }

        nextDeadlineMs += intervalMs;
        if (nextDeadlineMs > nowMs)
            return;

        // Skip elapsed deadlines so an overrun can never cause catch-up frames.
        const auto elapsedIntervals = std::floor((nowMs - nextDeadlineMs) / intervalMs) + 1.0;
        nextDeadlineMs += elapsedIntervals * intervalMs;
    }

private:
    double nextDeadlineMs = 0.0;
    double intervalMs = 1.0;
};
