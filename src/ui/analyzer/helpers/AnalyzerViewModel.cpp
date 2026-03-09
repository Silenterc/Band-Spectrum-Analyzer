#include "AnalyzerViewModel.h"

#include <algorithm>
#include <array>

AnalyzerViewModel::AnalyzerViewModel()
    : hoverModel(geometry, formatter, musicTheory) {
}

void AnalyzerViewModel::update(const Analyzer::CompositeSnapshot &snapshot, const AnalyzerViewState &viewState,
                               float gridMinDb, float gridMaxDb, float gridStepDb,
                               const juce::Rectangle<float> &localBounds,
                               const std::optional<juce::Point<float>> &hoverPositionToUse) {
    currentGridMinDb = gridMinDb;
    plotBounds = geometry.getPlotBounds(localBounds);
    updateVisibleFrequencyRange(snapshot, viewState);
    updateGrid(gridMinDb, gridMaxDb, gridStepDb);
    updateTraceVisuals(snapshot, viewState, gridMinDb, gridMaxDb, hoverPositionToUse);

    if (hoverPositionToUse.has_value()) {
        const auto primaryTrace = getPrimaryVisibleTrace(snapshot, viewState);

        if (primaryTrace.has_value()) {
            hoverInfo = hoverModel.build(localBounds, plotBounds, snapshot.bandInfo, primaryTrace->frame, gridMinDb,
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

    static constexpr std::array<float, 10> frequencyLabelsHz{
        20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f
    };

    for (auto frequencyHz: frequencyLabelsHz) {
        AnalyzerFrequencyMarker frequencyMarker;
        frequencyMarker.x = geometry.xForFrequency(frequencyHz, visibleMinFrequencyHz, visibleMaxFrequencyHz, plotBounds);
        frequencyMarker.label = formatter.formatScaleFrequency(frequencyHz);
        frequencyMarkers.push_back(frequencyMarker);
    }
}

void AnalyzerViewModel::updateTraceVisuals(const Analyzer::CompositeSnapshot &snapshot, const AnalyzerViewState &viewState,
                                           float gridMinDb, float gridMaxDb,
                                           const std::optional<juce::Point<float>> &hoverPositionToUse) {
    traceVisuals.clear();

    const auto hoveredBandIndex = hoverPositionToUse.has_value()
                                      ? geometry.bandIndexAt(*hoverPositionToUse, snapshot.bandInfo.size(), plotBounds)
                                      : -1;

    for (const auto &trace: snapshot.traces) {
        if (!isTraceEnabled(trace.kind, viewState))
            continue;

        AnalyzerTraceVisual traceVisual;
        traceVisual.kind = trace.kind;
        traceVisual.bars.resize(snapshot.bandInfo.size());

        for (size_t bandIndex = 0; bandIndex < snapshot.bandInfo.size(); ++bandIndex) {
            AnalyzerBarModel barModel;
            barModel.displayedDb = getDisplayedLevelDb(bandIndex, trace.frame, gridMinDb);
            barModel.bounds = geometry.getBarBounds(bandIndex, snapshot.bandInfo.size(), barModel.displayedDb, gridMinDb,
                                                    gridMaxDb, plotBounds);
            barModel.isHovered = hoveredBandIndex == static_cast<int>(bandIndex);
            traceVisual.bars[bandIndex] = barModel;
        }

        traceVisuals.push_back(std::move(traceVisual));
    }
}

std::optional<Analyzer::TraceSnapshot> AnalyzerViewModel::getPrimaryVisibleTrace(const Analyzer::CompositeSnapshot &snapshot,
                                                                                 const AnalyzerViewState &viewState) const {
    for (const auto &trace: snapshot.traces) {
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

float AnalyzerViewModel::getDisplayedLevelDb(size_t bandIndex, const Analyzer::Frame &latestFrame, float gridMinDb) {
    if (bandIndex >= latestFrame.peakDb.size() || bandIndex >= latestFrame.rmsDb.size())
        return gridMinDb;

    return std::max(latestFrame.peakDb[bandIndex], latestFrame.rmsDb[bandIndex]);
}

void AnalyzerViewModel::updateVisibleFrequencyRange(const Analyzer::CompositeSnapshot &snapshot,
                                                    const AnalyzerViewState &viewState) {
    if (snapshot.bandInfo.empty()) {
        visibleMinFrequencyHz = 20.0f;
        visibleMaxFrequencyHz = 20000.0f;
        return;
    }

    const auto fullMinFrequencyHz = snapshot.bandInfo.front().lowHz;
    const auto fullMaxFrequencyHz = snapshot.bandInfo.back().highHz;

    if (!viewState.useCustomFrequencyRange) {
        visibleMinFrequencyHz = fullMinFrequencyHz;
        visibleMaxFrequencyHz = fullMaxFrequencyHz;
        return;
    }

    visibleMinFrequencyHz = juce::jlimit(fullMinFrequencyHz, fullMaxFrequencyHz, viewState.visibleMinFrequencyHz);
    visibleMaxFrequencyHz = juce::jlimit(visibleMinFrequencyHz, fullMaxFrequencyHz, viewState.visibleMaxFrequencyHz);
}
