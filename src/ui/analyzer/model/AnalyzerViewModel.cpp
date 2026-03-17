#include "AnalyzerViewModel.h"

#include <algorithm>

#include "../../UiTheme.h"

AnalyzerViewModel::AnalyzerViewModel()
    : hoverModel(geometry, formatter, musicTheory) {
}

void AnalyzerViewModel::updateStaticLayout(const Analyzer::RenderData &renderData, const AnalyzerViewState &viewState,
                                           float gridMinDb, float gridMaxDb, float gridStepDb,
                                           const juce::Rectangle<float> &localBounds) {
    currentGridMinDb = gridMinDb;
    plotBounds = geometry.getPlotBounds(localBounds);
    updateVisibleFrequencyRange(renderData, viewState);
    updateGrid(gridMinDb, gridMaxDb, gridStepDb);
    updateBandBounds(renderData.bandInfo.size());
}

void AnalyzerViewModel::updateTraceVisuals(const Analyzer::RenderData &renderData, const AnalyzerViewState &viewState,
                                           const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                           const Shared::SignalSlotOrder &signalSlotOrder,
                                           float gridMinDb, float gridMaxDb) {
    traceVisuals.clear();

    std::vector<const Analyzer::RenderTrace *> orderedTraces;
    orderedTraces.reserve(renderData.traces.size());

    for (const auto &trace: renderData.traces) {
        if (!isTraceEnabled(trace.kind, viewState))
            continue;

        orderedTraces.push_back(&trace);
    }

    std::sort(orderedTraces.begin(), orderedTraces.end(),
              [this, &signalSlotOrder](const Analyzer::RenderTrace *lhs, const Analyzer::RenderTrace *rhs) {
                  return slotOrderModel.getTraceOrder(lhs->kind, signalSlotOrder)
                         < slotOrderModel.getTraceOrder(rhs->kind, signalSlotOrder);
              });

    const auto plotBottom = plotBounds.getBottom();

    for (const auto *trace: orderedTraces) {
        if (trace == nullptr)
            continue;

        AnalyzerTraceVisual traceVisual;
        traceVisual.kind = trace->kind;
        if (const auto slotIndex = Analyzer::slotIndexForTraceKind(trace->kind); slotIndex.has_value()) {
            const auto &slot = signalSlots[*slotIndex];
            traceVisual.colour = Ui::getSignalPresetColour(slot.colourIndex).withAlpha(slot.opacity);
        } else {
            traceVisual.colour = juce::Colours::white;
        }
        traceVisual.bars.resize(renderData.bandInfo.size());

        for (size_t bandIndex = 0; bandIndex < renderData.bandInfo.size(); ++bandIndex) {
            AnalyzerBarModel barModel;
            barModel.bandBounds = bandBounds[bandIndex];
            barModel.rmsDb = getRmsDb(bandIndex, trace->frame, gridMinDb);
            barModel.peakDb = getPeakDb(bandIndex, trace->frame, gridMinDb);
            barModel.holdDb = getHoldDb(bandIndex, trace->frame, gridMinDb);
            barModel.peakY = geometry.yForDb(barModel.peakDb, gridMinDb, gridMaxDb, plotBounds);
            barModel.holdY = geometry.yForDb(barModel.holdDb, gridMinDb, gridMaxDb, plotBounds);
            barModel.peakBounds = {barModel.bandBounds.getX(), barModel.peakY,
                                   barModel.bandBounds.getWidth(), plotBottom - barModel.peakY};
            const auto rmsY = geometry.yForDb(barModel.rmsDb, gridMinDb, gridMaxDb, plotBounds);
            barModel.rmsBounds = {barModel.bandBounds.getX(), rmsY,
                                  barModel.bandBounds.getWidth(), plotBottom - rmsY};
            traceVisual.bars[bandIndex] = barModel;
        }

        traceVisuals.push_back(std::move(traceVisual));
    }
}

void AnalyzerViewModel::updateHover(const Analyzer::RenderData &renderData, const AnalyzerViewState &viewState,
                                    const Shared::SignalSlotOrder &signalSlotOrder,
                                    const Analyzer::MeterSettings &meterSettings,
                                    float gridMinDb,
                                    const juce::Rectangle<float> &localBounds,
                                    const std::optional<juce::Point<float>> &hoverPositionToUse) {
    if (!hoverPositionToUse.has_value()) {
        hoverInfo.reset();
        return;
    }

    // Hover uses the first visible trace as its source until we add multi-trace hover policy
    const auto *primaryTrace = getPrimaryVisibleTrace(renderData, viewState, signalSlotOrder);

    if (primaryTrace == nullptr) {
        hoverInfo.reset();
        return;
    }

    hoverInfo = hoverModel.build(localBounds, plotBounds, renderData.bandInfo, primaryTrace->frame,
                                 meterSettings, gridMinDb, visibleMinFrequencyHz, visibleMaxFrequencyHz,
                                 *hoverPositionToUse);
}

const juce::Rectangle<float> &AnalyzerViewModel::getPlotBounds() const {
    return plotBounds;
}

const std::vector<AnalyzerGridLine> &AnalyzerViewModel::getGridLines() const {
    return gridLines;
}

const std::vector<AnalyzerFrequencyMarker> &AnalyzerViewModel::getFrequencyMarkers() const {
    return frequencyMarkers;
}

const std::vector<AnalyzerTraceVisual> &AnalyzerViewModel::getTraceVisuals() const {
    return traceVisuals;
}

const std::optional<AnalyzerHoverInfo> &AnalyzerViewModel::getHoverInfo() const {
    return hoverInfo;
}

float AnalyzerViewModel::getGridMinDb() const {
    return currentGridMinDb;
}

std::optional<juce::Rectangle<float>> AnalyzerViewModel::getBandBounds(const size_t bandIndex) const {
    if (bandIndex >= bandBounds.size())
        return std::nullopt;

    return bandBounds[bandIndex];
}

void AnalyzerViewModel::updateGrid(float gridMinDb, float gridMaxDb, float gridStepDb) {
    gridLines.clear();
    frequencyMarkers.clear();
    gridLines.reserve(static_cast<size_t>(std::ceil((gridMaxDb - gridMinDb) / gridStepDb)) + 1);
    frequencyMarkers.reserve(Ui::AnalyzerConstants::frequencyScaleLabelsHz.size());

    for (float db = gridMinDb; db <= gridMaxDb + 0.001f; db += gridStepDb) {
        AnalyzerGridLine gridLine;
        gridLine.y = geometry.yForDb(db, gridMinDb, gridMaxDb, plotBounds);
        gridLine.label = juce::String(static_cast<int>(std::round(db)));
        gridLines.push_back(gridLine);
    }

    for (auto frequencyHz: Ui::AnalyzerConstants::frequencyScaleLabelsHz) {
        AnalyzerFrequencyMarker frequencyMarker;
        frequencyMarker.x = geometry.xForFrequency(frequencyHz, visibleMinFrequencyHz, visibleMaxFrequencyHz, plotBounds);
        frequencyMarker.label = formatter.formatScaleFrequency(frequencyHz);
        frequencyMarkers.push_back(frequencyMarker);
    }
}

void AnalyzerViewModel::updateBandBounds(const size_t bandCount) {
    bandBounds.clear();
    bandBounds.reserve(bandCount);

    if (bandCount == 0)
        return;

    const auto bandWidth = plotBounds.getWidth() / static_cast<float>(bandCount);

    for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
        const auto x = plotBounds.getX() + static_cast<float>(bandIndex) * bandWidth;
        bandBounds.emplace_back(x + 1.0f, plotBounds.getY(), bandWidth - 2.0f, plotBounds.getHeight());
    }
}

const Analyzer::RenderTrace *AnalyzerViewModel::getPrimaryVisibleTrace(const Analyzer::RenderData &renderData,
                                                                       const AnalyzerViewState &viewState,
                                                                       const Shared::SignalSlotOrder &signalSlotOrder) const {
    for (const auto slotIndex: signalSlotOrder) {
        const auto kind = Analyzer::traceKindForSlot(slotIndex);
        if (!isTraceEnabled(kind, viewState))
            continue;

        const auto traceIterator = std::find_if(renderData.traces.begin(), renderData.traces.end(),
                                                [kind](const Analyzer::RenderTrace &trace) {
                                                    return trace.kind == kind;
                                                });
        if (traceIterator != renderData.traces.end())
            return &*traceIterator;
    }

    for (const auto &trace: renderData.traces) {
        if (isTraceEnabled(trace.kind, viewState))
            return &trace;
    }

    return nullptr;
}

bool AnalyzerViewModel::isTraceEnabled(Analyzer::TraceKind kind, const AnalyzerViewState &viewState) const {
    if (viewState.enabledTraces.empty())
        return false;

    return std::find(viewState.enabledTraces.begin(), viewState.enabledTraces.end(), kind) != viewState.enabledTraces.end();
}

float AnalyzerViewModel::getRmsDb(size_t bandIndex, const Analyzer::RenderFrame &renderFrame, float gridMinDb) {
    if (bandIndex >= renderFrame.rmsDb.size())
        return gridMinDb;

    return renderFrame.rmsDb[bandIndex];
}

float AnalyzerViewModel::getPeakDb(size_t bandIndex, const Analyzer::RenderFrame &renderFrame, float gridMinDb) {
    if (bandIndex >= renderFrame.peakDb.size())
        return gridMinDb;

    return renderFrame.peakDb[bandIndex];
}

float AnalyzerViewModel::getHoldDb(size_t bandIndex, const Analyzer::RenderFrame &renderFrame, float gridMinDb) {
    if (bandIndex >= renderFrame.holdDb.size())
        return gridMinDb;

    return renderFrame.holdDb[bandIndex];
}

void AnalyzerViewModel::updateVisibleFrequencyRange(const Analyzer::RenderData &renderData,
                                                    const AnalyzerViewState &viewState) {
    if (renderData.bandInfo.empty()) {
        visibleMinFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMinFrequencyHz;
        visibleMaxFrequencyHz = Ui::AnalyzerConstants::defaultVisibleMaxFrequencyHz;
        return;
    }

    const auto fullMinFrequencyHz = renderData.bandInfo.front().lowHz;
    const auto fullMaxFrequencyHz = renderData.bandInfo.back().highHz;

    if (!viewState.useCustomFrequencyRange) {
        visibleMinFrequencyHz = fullMinFrequencyHz;
        visibleMaxFrequencyHz = fullMaxFrequencyHz;
        return;
    }

    visibleMinFrequencyHz = juce::jlimit(fullMinFrequencyHz, fullMaxFrequencyHz, viewState.visibleMinFrequencyHz);
    visibleMaxFrequencyHz = juce::jlimit(visibleMinFrequencyHz, fullMaxFrequencyHz, viewState.visibleMaxFrequencyHz);
}
