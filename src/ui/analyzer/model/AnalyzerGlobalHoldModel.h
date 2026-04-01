#pragma once

#include <optional>
#include <vector>

#include "../AnalyzerRenderData.h"
#include "../../SignalSlotUiState.h"

struct AnalyzerGlobalHoldFrame {
    std::vector<float> holdDb;
    std::vector<std::optional<Analyzer::TraceKind>> ownerKinds;
};

class AnalyzerGlobalHoldModel final {
public:
    void reset();

    void tick(const Analyzer::RenderData &renderData,
              const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
              const Analyzer::MeterSettings &meterSettings,
              float floorDb,
              float dtSeconds);

    const std::optional<AnalyzerGlobalHoldFrame> &getFrame() const;
    bool isSettledAtFloor(float floorDb) const;

private:
    void ensureFrame(size_t bandCount, float floorDb);

    std::optional<AnalyzerGlobalHoldFrame> frame;
    std::vector<float> holdTimeRemainingMs;
};
