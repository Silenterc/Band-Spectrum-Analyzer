#include "AnalyzerViewModel.h"

#include <algorithm>

AnalyzerViewModel::AnalyzerViewModel()
    : hoverModel(geometry, formatter, musicTheory) {
}

void AnalyzerViewModel::update(const Analyzer::RenderData &renderData, const AnalyzerViewState &viewState,
                               float gridMinDb, float gridMaxDb, float gridStepDb,
                               const juce::Rectangle<float> &localBounds,
                               const std::optional<juce::Point<float>> &hoverPositionToUse) {
    currentGridMinDb = gridMinDb;
    plotBounds = geometry.getPlotBounds(localBounds);
    updateVisibleFrequencyRange(renderData, viewState);
    updateGrid(gridMinDb, gridMaxDb, gridStepDb);
    updateTraceVisuals(renderData, viewState, gridMinDb, gridMaxDb, hoverPositionToUse);

    if (hoverPositionToUse.has_value()) {
        const auto primaryTrace = getPrimaryVisibleTrace(renderData, viewState);

        if (primaryTrace.has_value()) {
            hoverInfo = hoverModel.build(localBounds, plotBounds, renderData.bandInfo, primaryTrace->frame, gridMinDb,
                                         visibleMinFrequencyHz, visibleMaxFrequencyHz, *hoverPositionToUse);
        } else {
            hoverInfo.reset();
        }
    } else {
        hoverInfo.reset();
    }
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

void AnalyzerViewModel::updateGrid(float gridMinDb, float gridMaxDb, float gridStepDb) {
    gridLines.clear();
    frequencyMarkers.clear();

    for (float db = gridMinDb; db <= gridMaxDb + 0.001f; db += gridStepDb) {
        AnalyzerGridLine gridLine;
        gridLine.y = geometry.yForDb(db, gridMinDb, gridMaxDb, plotBounds);
        gridLine.label = juce::String(static_cast<int>(std::round(db)));
        gridLines.push_back(gridLine);
    }

    for (auto frequencyHz: Analyzer::Constants::frequencyScaleLabelsHz) {
        AnalyzerFrequencyMarker frequencyMarker;
        frequencyMarker.x = geometry.xForFrequency(frequencyHz, visibleMinFrequencyHz, visibleMaxFrequencyHz, plotBounds);
        frequencyMarker.label = formatter.formatScaleFrequency(frequencyHz);
        frequencyMarkers.push_back(frequencyMarker);
    }
}

void AnalyzerViewModel::updateTraceVisuals(const Analyzer::RenderData &renderData, const AnalyzerViewState &viewState,
                                           float gridMinDb, float gridMaxDb,
                                           const std::optional<juce::Point<float>> &hoverPositionToUse) {
    traceVisuals.clear();

    const auto hoveredBandIndex = hoverPositionToUse.has_value()
                                      ? geometry.bandIndexAt(*hoverPositionToUse, renderData.bandInfo.size(), plotBounds)
                                      : -1;

    for (const auto &trace: renderData.traces) {
        if (!isTraceEnabled(trace.kind, viewState))
            continue;

        AnalyzerTraceVisual traceVisual;
        traceVisual.kind = trace.kind;
        traceVisual.bars.resize(renderData.bandInfo.size());

        for (size_t bandIndex = 0; bandIndex < renderData.bandInfo.size(); ++bandIndex) {
            AnalyzerBarModel barModel;
            barModel.rmsDb = getRmsDb(bandIndex, trace.frame, gridMinDb);
            barModel.peakDb = getPeakDb(bandIndex, trace.frame, gridMinDb);
            barModel.rmsBounds = geometry.getBarBounds(bandIndex, renderData.bandInfo.size(), barModel.rmsDb, gridMinDb,
                                                       gridMaxDb, plotBounds);
            barModel.peakY = geometry.yForDb(barModel.peakDb, gridMinDb, gridMaxDb, plotBounds);
            barModel.isHovered = hoveredBandIndex == static_cast<int>(bandIndex);
            traceVisual.bars[bandIndex] = barModel;
        }

        traceVisuals.push_back(std::move(traceVisual));
    }
}

std::optional<Analyzer::RenderTrace> AnalyzerViewModel::getPrimaryVisibleTrace(const Analyzer::RenderData &renderData,
                                                                               const AnalyzerViewState &viewState) const {
    for (const auto &trace: renderData.traces) {
        if (isTraceEnabled(trace.kind, viewState))
            return trace;
    }

    return std::nullopt;
}

bool AnalyzerViewModel::isTraceEnabled(Analyzer::TraceKind kind, const AnalyzerViewState &viewState) const {
    if (viewState.enabledTraces.empty())
        return true;

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

void AnalyzerViewModel::updateVisibleFrequencyRange(const Analyzer::RenderData &renderData,
                                                    const AnalyzerViewState &viewState) {
    if (renderData.bandInfo.empty()) {
        visibleMinFrequencyHz = Analyzer::Constants::defaultVisibleMinFrequencyHz;
        visibleMaxFrequencyHz = Analyzer::Constants::defaultVisibleMaxFrequencyHz;
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
