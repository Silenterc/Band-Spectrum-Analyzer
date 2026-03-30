#include "AnalyzerComponent.h"

#include <algorithm>
#include <cmath>

namespace {
    bool nearlyEqual(const float lhs, const float rhs) {
        return std::abs(lhs - rhs) <= 0.0001f;
    }
}

bool AnalyzerComponent::StaticViewStateKey::operator==(const StaticViewStateKey &other) const {
    return bounds == other.bounds
           && bandCount == other.bandCount
           && nearlyEqual(minBandFrequencyHz, other.minBandFrequencyHz)
           && nearlyEqual(maxBandFrequencyHz, other.maxBandFrequencyHz)
           && nearlyEqual(gridMinDb, other.gridMinDb)
           && nearlyEqual(gridMaxDb, other.gridMaxDb)
           && nearlyEqual(gridStepDb, other.gridStepDb)
           && useCustomFrequencyRange == other.useCustomFrequencyRange
           && nearlyEqual(visibleMinFrequencyHz, other.visibleMinFrequencyHz)
           && nearlyEqual(visibleMaxFrequencyHz, other.visibleMaxFrequencyHz);
}

AnalyzerComponent::AnalyzerComponent(AnalyzerRenderSource &renderSourceToUse,
                                     AnalyzerUiSnapshotSource &snapshotSourceToUse,
                                     const Ui::Theme &themeToUse)
    : renderSource(renderSourceToUse),
      snapshotSource(snapshotSourceToUse),
      theme(themeToUse),
      viewModel(themeToUse),
      hoverOverlay(themeToUse) {
    setOpaque(false);
    addAndMakeVisible(hoverOverlay);

    uiSnapshot = snapshotSource.getAnalyzerUiSnapshot();
    snapshotSource.addAnalyzerUiSnapshotListener(*this);
    bandInfo = renderSource.getBandInfo();
    rawTraces = renderSource.getRawTraces();
    // Prime the meter so the first paint already has render-ready values
    displayMeter.tick(bandInfo, rawTraces, uiSnapshot.meterSettings, uiSnapshot.gridMinDb, Ui::AnalyzerConstants::meterPollIntervalSeconds);
    renderData = composeDisplayRenderData(displayMeter.getRenderData());
    lastPaintedRenderData = renderData;
    refreshModel.prime(uiSnapshot);
    rebuildViewModels();
    startTimer(Ui::AnalyzerConstants::meterPollIntervalMs);
}

AnalyzerComponent::~AnalyzerComponent() {
    snapshotSource.removeAnalyzerUiSnapshotListener(*this);
}

void AnalyzerComponent::paint(juce::Graphics &g) {
    if (refreshModel.syncFreezeEdge(uiSnapshot, renderData, lastPaintedRenderData)) {
        rebuildViewModels();
    }

    ensureStaticLayer();
    g.drawImageAt(staticLayer, 0, 0);
    drawBars(g);

    lastPaintedRenderData = renderData;
}

void AnalyzerComponent::resized() {
    staticLayerDirty = true;
    hoverOverlay.setBounds(getLocalBounds());
    rebuildViewModels();
}

void AnalyzerComponent::mouseMove(const juce::MouseEvent &event) {
    hoverPosition = event.position;
    hoverUpdatePending = true;
}

void AnalyzerComponent::mouseDrag(const juce::MouseEvent &event) {
    hoverPosition = event.position;
    hoverUpdatePending = true;
}

void AnalyzerComponent::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hoverPosition.reset();
    hoverUpdatePending = true;
}

void AnalyzerComponent::drawGrid(juce::Graphics &g) const {
    const auto plotBounds = viewModel.getPlotBounds();
    const auto &plotMetrics = theme.metrics.analyzerPlot;
    const auto plotFrameBounds = plotBounds.expanded(plotMetrics.frameExpansion);

    g.setColour(theme.gridBorder);
    g.drawRoundedRectangle(plotFrameBounds, plotMetrics.frameCornerRadius, 1.0f);

    for (const auto &gridLine: viewModel.getGridLines()) {
        g.setColour(theme.gridLine.withMultipliedAlpha(0.9f));
        g.drawHorizontalLine(static_cast<int>(std::round(gridLine.y)), plotBounds.getX(), plotBounds.getRight());

        g.setColour(theme.axisText);
        g.setFont(plotMetrics.gridLabelFontHeight);
        g.drawText(gridLine.label,
                   0,
                   static_cast<int>(gridLine.y - plotMetrics.gridLabelYOffset),
                   plotMetrics.gridLabelWidth,
                   plotMetrics.gridLabelHeight,
                   juce::Justification::centredRight);
    }

    for (const auto &frequencyMarker: viewModel.getFrequencyMarkers()) {
        g.setColour(theme.gridLine.withMultipliedAlpha(0.62f));
        g.drawVerticalLine(static_cast<int>(std::round(frequencyMarker.x)), plotBounds.getY(), plotBounds.getBottom());

        g.setColour(theme.axisText);
        g.drawText(frequencyMarker.label,
                   static_cast<int>(frequencyMarker.x - plotMetrics.frequencyLabelXHalfSpan),
                   static_cast<int>(plotBounds.getBottom() + plotMetrics.frequencyLabelYOffset),
                   plotMetrics.frequencyLabelWidth,
                   plotMetrics.frequencyLabelHeight,
                   juce::Justification::centred);
    }
}

void AnalyzerComponent::drawBars(juce::Graphics &g) const {
    const auto clipBounds = g.getClipBounds().toFloat();

    for (const auto &traceVisual: viewModel.getTraceVisuals()) {
        const auto peakColour = traceVisual.colour;
        const auto rmsColour = peakColour.withMultipliedAlpha(0.45f);
        const auto lineColour = Ui::makeHoldIndicatorColour(peakColour, theme);

        g.setColour(peakColour);
        for (size_t bandIndex = 0; bandIndex < traceVisual.bars.size(); ++bandIndex) {
            const auto &bar = traceVisual.bars[bandIndex];
            if (!bar.bandBounds.intersects(clipBounds) || bar.peakDb <= viewModel.getGridMinDb())
                continue;

            const auto peakBounds = bar.peakBounds.getSmallestIntegerContainer();
            if (!peakBounds.isEmpty())
                g.fillRect(peakBounds);
        }

        g.setColour(rmsColour);
        for (size_t bandIndex = 0; bandIndex < traceVisual.bars.size(); ++bandIndex) {
            const auto &bar = traceVisual.bars[bandIndex];
            if (!bar.bandBounds.intersects(clipBounds) || bar.rmsDb <= viewModel.getGridMinDb())
                continue;

            const auto rmsBounds = bar.rmsBounds.getSmallestIntegerContainer();
            if (!rmsBounds.isEmpty())
                g.fillRect(rmsBounds);
        }

        g.setColour(lineColour);
        for (size_t bandIndex = 0; bandIndex < traceVisual.bars.size(); ++bandIndex) {
            const auto &bar = traceVisual.bars[bandIndex];
            if (!bar.bandBounds.intersects(clipBounds))
                continue;

            if (bar.holdDb <= viewModel.getGridMinDb())
                continue;

            const auto lineBounds = juce::Rectangle<float>(bar.bandBounds.getX(), bar.holdY - 1.0f,
                                                           bar.bandBounds.getWidth(), 2.0f).getSmallestIntegerContainer();
            if (!lineBounds.isEmpty())
                g.fillRect(lineBounds);
        }
    }
}

void AnalyzerComponent::syncFrozenSlotCache(
    const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &previousSignalSlots) {
    for (size_t slotIndex = 0; slotIndex < uiSnapshot.signalSlots.size(); ++slotIndex) {
        const auto &previousSlot = previousSignalSlots[slotIndex];
        const auto &currentSlot = uiSnapshot.signalSlots[slotIndex];

        if (!currentSlot.configuration.enabled || !currentSlot.frozen) {
            clearFrozenTrace(slotIndex);
            continue;
        }

        if (!previousSlot.frozen && currentSlot.frozen)
            captureFrozenTrace(slotIndex, lastPaintedRenderData);
    }
}

Analyzer::RenderData AnalyzerComponent::composeDisplayRenderData(const Analyzer::RenderData &liveRenderData) {
    Analyzer::RenderData displayRenderData;
    displayRenderData.bandInfo = liveRenderData.bandInfo;
    displayRenderData.traces.reserve(liveRenderData.traces.size());

    std::array<bool, Shared::maxSignalSlots> hasLiveTrace{};

    for (const auto &liveTrace: liveRenderData.traces) {
        if (const auto slotIndex = Analyzer::slotIndexForTraceKind(liveTrace.kind); slotIndex.has_value()) {
            hasLiveTrace[*slotIndex] = true;

            if (!uiSnapshot.signalSlots[*slotIndex].configuration.enabled) {
                clearFrozenTrace(*slotIndex);
                continue;
            }

            if (uiSnapshot.signalSlots[*slotIndex].frozen) {
                // Individual slot freeze is a pure UI concern: keep showing the cached
                // rendered trace while the live meter continues updating in the background.
                if (frozenSlotTraces[*slotIndex].has_value()
                    && isTraceCompatible(*frozenSlotTraces[*slotIndex], displayRenderData.bandInfo.size())) {
                    displayRenderData.traces.push_back(*frozenSlotTraces[*slotIndex]);
                } else {
                    frozenSlotTraces[*slotIndex] = liveTrace;
                    displayRenderData.traces.push_back(liveTrace);
                }
                continue;
            }

            displayRenderData.traces.push_back(liveTrace);
            continue;
        }

        displayRenderData.traces.push_back(liveTrace);
    }

    for (size_t slotIndex = 0; slotIndex < uiSnapshot.signalSlots.size(); ++slotIndex) {
        const auto &slot = uiSnapshot.signalSlots[slotIndex];
        if (!slot.configuration.enabled || !slot.frozen || hasLiveTrace[slotIndex])
            continue;

        if (frozenSlotTraces[slotIndex].has_value()
            && isTraceCompatible(*frozenSlotTraces[slotIndex], displayRenderData.bandInfo.size())) {
            displayRenderData.traces.push_back(*frozenSlotTraces[slotIndex]);
        }
    }

    return displayRenderData;
}

void AnalyzerComponent::captureFrozenTrace(const size_t slotIndex, const Analyzer::RenderData &sourceRenderData) {
    if (slotIndex >= frozenSlotTraces.size())
        return;

    if (const auto trace = findTrace(sourceRenderData, Analyzer::traceKindForSlot(slotIndex)); trace.has_value())
        frozenSlotTraces[slotIndex] = *trace;
}

void AnalyzerComponent::clearFrozenTrace(const size_t slotIndex) {
    if (slotIndex >= frozenSlotTraces.size())
        return;

    frozenSlotTraces[slotIndex].reset();
}

std::optional<Analyzer::RenderTrace> AnalyzerComponent::findTrace(const Analyzer::RenderData &sourceRenderData,
                                                                  const Analyzer::TraceKind kind) {
    const auto iterator = std::find_if(sourceRenderData.traces.begin(), sourceRenderData.traces.end(),
                                       [kind](const Analyzer::RenderTrace &trace) {
                                           return trace.kind == kind;
                                       });
    if (iterator == sourceRenderData.traces.end())
        return std::nullopt;

    return *iterator;
}

bool AnalyzerComponent::isTraceCompatible(const Analyzer::RenderTrace &trace, const size_t bandCount) {
    return trace.frame.rmsDb.size() == bandCount
           && trace.frame.peakDb.size() == bandCount
           && trace.frame.holdDb.size() == bandCount;
}

void AnalyzerComponent::rebuildViewModels() {
    refreshStaticViewModelIfNeeded();
    rebuildDynamicViewModel();
    updateHoverState();
}

void AnalyzerComponent::refreshStaticViewModelIfNeeded() {
    StaticViewStateKey nextKey;
    nextKey.bounds = getLocalBounds();
    nextKey.bandCount = renderData.bandInfo.size();
    nextKey.gridMinDb = uiSnapshot.gridMinDb;
    nextKey.gridMaxDb = uiSnapshot.gridMaxDb;
    nextKey.gridStepDb = uiSnapshot.gridStepDb;
    nextKey.useCustomFrequencyRange = viewState.useCustomFrequencyRange;
    nextKey.visibleMinFrequencyHz = viewState.visibleMinFrequencyHz;
    nextKey.visibleMaxFrequencyHz = viewState.visibleMaxFrequencyHz;

    if (!renderData.bandInfo.empty()) {
        nextKey.minBandFrequencyHz = renderData.bandInfo.front().lowHz;
        nextKey.maxBandFrequencyHz = renderData.bandInfo.back().highHz;
    }

    if (staticViewStateKey.has_value() && *staticViewStateKey == nextKey)
        return;

    staticViewStateKey = nextKey;
    staticLayerDirty = true;
    viewModel.updateStaticLayout(renderData, viewState, uiSnapshot.gridMinDb, uiSnapshot.gridMaxDb,
                                 uiSnapshot.gridStepDb, getLocalBounds().toFloat());
}

void AnalyzerComponent::rebuildDynamicViewModel() {
    viewModel.updateTraceVisuals(renderData, viewState, uiSnapshot.signalSlots, uiSnapshot.slotOrder,
                                 uiSnapshot.gridMinDb, uiSnapshot.gridMaxDb);
}

void AnalyzerComponent::updateHoverState() {
    viewModel.updateHover(renderData, uiSnapshot.gridMinDb, uiSnapshot.gridMaxDb, getLocalBounds().toFloat(), hoverPosition);
    std::optional<juce::Rectangle<float>> hoveredBandBounds;
    if (const auto &hoverInfo = viewModel.getHoverInfo(); hoverInfo.has_value())
        hoveredBandBounds = viewModel.getBandBounds(hoverInfo->bandIndex);

    hoverOverlay.updateState(viewModel.getHoverInfo(),
                             viewModel.getTraceVisuals(),
                             viewModel.getGridMinDb(),
                             hoveredBandBounds);
}

void AnalyzerComponent::ensureStaticLayer() {
    if (!staticLayerDirty)
        return;

    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        staticLayer = {};
        staticLayerDirty = false;
        return;
    }

    staticLayer = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics layerGraphics(staticLayer);
    const auto plotBounds = viewModel.getPlotBounds();
    const auto &plotMetrics = theme.metrics.analyzerPlot;
    const auto plotFrameBounds = plotBounds.expanded(plotMetrics.frameExpansion);
    juce::ColourGradient plotGradient(
        theme.plotBackground.brighter(plotMetrics.gradientTopBrightness),
        plotBounds.getCentreX(),
        plotBounds.getY(),
        theme.plotBackground.darker(plotMetrics.gradientBottomDarkness),
        plotBounds.getCentreX(),
        plotBounds.getBottom(),
        false);
    plotGradient.addColour(plotMetrics.gradientMidPoint, theme.plotBackground);
    layerGraphics.setGradientFill(plotGradient);
    layerGraphics.fillRoundedRectangle(plotFrameBounds, plotMetrics.frameCornerRadius);

    drawGrid(layerGraphics);
    staticLayerDirty = false;
}

void AnalyzerComponent::timerCallback() {
    processPendingHoverUpdate();

    const auto refreshDecision = refreshModel.makeTimerDecision(renderSource, bandInfo, displayMeter, uiSnapshot);
    if (refreshModel.syncFreezeEdge(uiSnapshot, renderData, lastPaintedRenderData))
        rebuildViewModels();
    if (refreshDecision.pollingIntervalChanged)
        startTimer(refreshDecision.pollIntervalMs);

    if (!refreshDecision.frozen) {
        bandInfo = refreshDecision.nextBandInfo;

        if (!refreshDecision.shouldAdvanceDisplay
            && !refreshDecision.bandLayoutChanged)
            return;

        if (refreshDecision.shouldAdvanceDisplay || refreshDecision.bandLayoutChanged) {
            rawTraces = renderSource.getRawTraces();
            // Raw DSP measurements become render-ready RMS, peak, and hold values here
            displayMeter.tick(bandInfo, rawTraces, uiSnapshot.meterSettings, uiSnapshot.gridMinDb, refreshDecision.dtSeconds);
        }

        renderData = composeDisplayRenderData(displayMeter.getRenderData());
    } else {
        bandInfo = refreshDecision.nextBandInfo;
    }

    rebuildViewModels();
    repaint();
}

void AnalyzerComponent::processPendingHoverUpdate() {
    if (!hoverUpdatePending)
        return;

    updateHoverState();
    hoverUpdatePending = false;
}

void AnalyzerComponent::analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) {
    if (uiSnapshot == snapshot)
        return;

    const auto previousSignalSlots = uiSnapshot.signalSlots;
    const auto wasFrozen = uiSnapshot.frozen;
    uiSnapshot = snapshot;
    syncFrozenSlotCache(previousSignalSlots);

    if (uiSnapshot.frozen && !wasFrozen) {
        renderData = lastPaintedRenderData;
    } else {
        renderData = composeDisplayRenderData(displayMeter.getRenderData());
    }

    rebuildViewModels();
    repaint();
}
