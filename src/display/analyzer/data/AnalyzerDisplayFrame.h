#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "display/analyzer/data/AnalyzerGlobalHoldFrame.h"
#include "display/analyzer/data/AnalyzerMeterData.h"
#include "shared/SignalSlotConfiguration.h"

struct AnalyzerSlotDisplayFrame {
    bool active = false;
    Analyzer::TraceKind kind = Analyzer::TraceKind::slot1;
    Analyzer::MeterFrame frame;
};

struct AnalyzerDisplayFrame {
    std::shared_ptr<const std::vector<Analyzer::BandInfo>> bandInfo;
    std::array<AnalyzerSlotDisplayFrame, Shared::maxSignalSlots> slotFrames{};
    std::optional<AnalyzerGlobalHoldFrame> globalHoldFrame;
    std::uint64_t revision = 0;
};
