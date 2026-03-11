#include "AnalyzerComponent.h"
#include "../../dsp/AnalyzerConstants.h"

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

AnalyzerComponent::AnalyzerComponent(AnalyzerDataSource &source, const Ui::Theme &themeToUse)
    : dataSource(source), theme(themeToUse) {
    setOpaque(true);

    refreshUiSnapshot();
    bandInfo = dataSource.getBandInfo();
    rawTraces = dataSource.getRawTraces();
    // Prime the meter so the first paint already has render-ready values
    displayMeter.tick(bandInfo, rawTraces, meterSettings, gridMinDb, Analyzer::Constants::meterPollIntervalSeconds);
    renderData = displayMeter.getRenderData();
    lastPaintedRenderData = renderData;
    wasFrozen = dataSource.isFrozen();
    lastPollTimeMs = juce::Time::getMillisecondCounterHiRes();
    rebuildEnabledTraces();
    refreshStaticViewModelIfNeeded();
    rebuildDynamicViewModel();
    updateHoverState();
    startTimer(Analyzer::Constants::meterPollIntervalMs);
}

void AnalyzerComponent::paint(juce::Graphics &g) {
    syncFreezeSnapshotIfNeeded();

    ensureStaticLayer();
    g.drawImageAt(staticLayer, 0, 0);
    drawBars(g);
    drawHoverInfo(g);

    lastPaintedRenderData = renderData;
}

void AnalyzerComponent::resized() {
    staticLayerDirty = true;
    refreshStaticViewModelIfNeeded();
    rebuildDynamicViewModel();
    updateHoverState();
}

void AnalyzerComponent::mouseMove(const juce::MouseEvent &event) {
    const auto previousHoverInfo = viewModel.getHoverInfo();
    hoverPosition = event.position;
    updateHoverState();
    repaintHoverDelta(previousHoverInfo);
}

void AnalyzerComponent::mouseDrag(const juce::MouseEvent &event) {
    const auto previousHoverInfo = viewModel.getHoverInfo();
    hoverPosition = event.position;
    updateHoverState();
    repaintHoverDelta(previousHoverInfo);
}

void AnalyzerComponent::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    const auto previousHoverInfo = viewModel.getHoverInfo();
    hoverPosition.reset();
    updateHoverState();
    repaintHoverDelta(previousHoverInfo);
}

void AnalyzerComponent::drawGrid(juce::Graphics &g) const {
    const auto plotBounds = viewModel.getPlotBounds();

    g.setColour(theme.gridBorder);
    g.drawRoundedRectangle(plotBounds.expanded(1.0f), 8.0f, 1.0f);

    for (const auto &gridLine: viewModel.getGridLines()) {
        g.setColour(theme.gridLine);
        g.drawHorizontalLine(static_cast<int>(std::round(gridLine.y)), plotBounds.getX(), plotBounds.getRight());

        g.setColour(theme.axisText);
        g.setFont(11.0f);
        g.drawText(gridLine.label, 0, static_cast<int>(gridLine.y - 7.0f), 48, 14, juce::Justification::centredRight);
    }

    for (const auto &frequencyMarker: viewModel.getFrequencyMarkers()) {
        g.setColour(theme.gridLine);
        g.drawVerticalLine(static_cast<int>(std::round(frequencyMarker.x)), plotBounds.getY(), plotBounds.getBottom());

        g.setColour(theme.axisText);
        g.drawText(frequencyMarker.label, static_cast<int>(frequencyMarker.x - 18.0f),
                   static_cast<int>(plotBounds.getBottom() + 6.0f),
                   36, 16, juce::Justification::centred);
    }
}

void AnalyzerComponent::drawBars(juce::Graphics &g) const {
    const auto hoveredBandIndex = viewModel.getHoverInfo().has_value()
                                      ? std::optional<size_t>{viewModel.getHoverInfo()->bandIndex}
                                      : std::nullopt;
    const auto clipBounds = g.getClipBounds().toFloat();

    for (const auto &traceVisual: viewModel.getTraceVisuals()) {
        const auto peakColour = traceVisual.colour;
        const auto rmsColour = peakColour.withMultipliedAlpha(0.45f);
        const auto lineColour = peakColour.brighter(0.25f);

        g.setColour(peakColour);
        for (size_t bandIndex = 0; bandIndex < traceVisual.bars.size(); ++bandIndex) {
            if (hoveredBandIndex.has_value() && bandIndex == *hoveredBandIndex)
                continue;

            const auto &bar = traceVisual.bars[bandIndex];
            if (!bar.bandBounds.intersects(clipBounds) || bar.peakDb <= viewModel.getGridMinDb())
                continue;

            const auto peakBounds = bar.peakBounds.getSmallestIntegerContainer();
            if (!peakBounds.isEmpty())
                g.fillRect(peakBounds);
        }

        g.setColour(rmsColour);
        for (size_t bandIndex = 0; bandIndex < traceVisual.bars.size(); ++bandIndex) {
            if (hoveredBandIndex.has_value() && bandIndex == *hoveredBandIndex)
                continue;

            const auto &bar = traceVisual.bars[bandIndex];
            if (!bar.bandBounds.intersects(clipBounds) || bar.rmsDb <= viewModel.getGridMinDb())
                continue;

            const auto rmsBounds = bar.rmsBounds.getSmallestIntegerContainer();
            if (!rmsBounds.isEmpty())
                g.fillRect(rmsBounds);
        }

        g.setColour(lineColour);
        for (size_t bandIndex = 0; bandIndex < traceVisual.bars.size(); ++bandIndex) {
            if (hoveredBandIndex.has_value() && bandIndex == *hoveredBandIndex)
                continue;

            const auto &bar = traceVisual.bars[bandIndex];
            if (!bar.bandBounds.intersects(clipBounds))
                continue;

            const auto lineDb = bar.holdDb > viewModel.getGridMinDb() ? bar.holdDb : bar.peakDb;
            const auto lineY = bar.holdDb > viewModel.getGridMinDb() ? bar.holdY : bar.peakY;
            if (lineDb <= viewModel.getGridMinDb())
                continue;

            const auto lineBounds = juce::Rectangle<float>(bar.bandBounds.getX(), lineY - 1.0f,
                                                           bar.bandBounds.getWidth(), 2.0f).getSmallestIntegerContainer();
            if (!lineBounds.isEmpty())
                g.fillRect(lineBounds);
        }

        if (!hoveredBandIndex.has_value() || *hoveredBandIndex >= traceVisual.bars.size())
            continue;

        const auto &hoveredBar = traceVisual.bars[*hoveredBandIndex];
        if (!hoveredBar.bandBounds.intersects(clipBounds))
            continue;

        const auto hoveredPeakColour = peakColour.brighter(0.18f);
        const auto hoveredRmsColour = hoveredPeakColour.withMultipliedAlpha(0.45f);
        const auto hoveredLineColour = hoveredPeakColour.brighter(0.1f);

        if (hoveredBar.peakDb > viewModel.getGridMinDb()) {
            g.setColour(hoveredPeakColour);
            const auto peakBounds = hoveredBar.peakBounds.getSmallestIntegerContainer();
            if (!peakBounds.isEmpty())
                g.fillRect(peakBounds);
        }

        if (hoveredBar.rmsDb > viewModel.getGridMinDb()) {
            g.setColour(hoveredRmsColour);
            const auto rmsBounds = hoveredBar.rmsBounds.getSmallestIntegerContainer();
            if (!rmsBounds.isEmpty())
                g.fillRect(rmsBounds);
        }

        const auto lineDb = hoveredBar.holdDb > viewModel.getGridMinDb() ? hoveredBar.holdDb : hoveredBar.peakDb;
        const auto lineY = hoveredBar.holdDb > viewModel.getGridMinDb() ? hoveredBar.holdY : hoveredBar.peakY;
        if (lineDb > viewModel.getGridMinDb()) {
            g.setColour(hoveredLineColour);
            const auto lineBounds = juce::Rectangle<float>(hoveredBar.bandBounds.getX(), lineY - 1.0f,
                                                           hoveredBar.bandBounds.getWidth(), 2.0f)
                                        .getSmallestIntegerContainer();
            if (!lineBounds.isEmpty())
                g.fillRect(lineBounds);
        }
    }
}

void AnalyzerComponent::drawHoverInfo(juce::Graphics &g) const {
    if (!viewModel.getHoverInfo().has_value())
        return;

    const auto &hoverInfo = *viewModel.getHoverInfo();

    g.setColour(theme.tooltipBackground);
    g.fillRoundedRectangle(hoverInfo.bounds, 8.0f);

    g.setColour(theme.tooltipBorder);
    g.drawRoundedRectangle(hoverInfo.bounds, 8.0f, 1.0f);

    juce::StringArray tooltipLines;

    if (hoverInfo.peakText.isNotEmpty())
        tooltipLines.add(hoverInfo.peakText);

    if (hoverInfo.rmsText.isNotEmpty())
        tooltipLines.add(hoverInfo.rmsText);

    tooltipLines.add(hoverInfo.frequencyText);
    tooltipLines.add(hoverInfo.noteText);

    const auto tooltipText = tooltipLines.joinIntoString("\n");

    g.setColour(theme.tooltipText);
    g.setFont(12.0f);
    g.drawFittedText(tooltipText, hoverInfo.bounds.toNearestInt().reduced(10, 8), juce::Justification::centredLeft, 3);
}

bool AnalyzerComponent::refreshUiSnapshot() {
    const auto previousSignalSlots = signalSlots;
    const auto previousSignalSlotOrder = signalSlotOrder;
    const auto previousMeterSettings = meterSettings;
    const auto previousGridMinDb = gridMinDb;
    const auto previousGridMaxDb = gridMaxDb;
    const auto previousGridStepDb = gridStepDb;

    signalSlots = dataSource.getSignalSlots();
    signalSlotOrder = dataSource.getSignalSlotOrder();
    meterSettings = dataSource.getMeterSettings();
    gridMinDb = dataSource.getGridMinDb();
    gridMaxDb = dataSource.getGridMaxDb();
    gridStepDb = dataSource.getGridStepDb();

    return signalSlots != previousSignalSlots
           || signalSlotOrder != previousSignalSlotOrder
           || meterSettings.showRms != previousMeterSettings.showRms
           || meterSettings.showPeak != previousMeterSettings.showPeak
           || meterSettings.showHold != previousMeterSettings.showHold
           || !nearlyEqual(meterSettings.holdMs, previousMeterSettings.holdMs)
           || !nearlyEqual(gridMinDb, previousGridMinDb)
           || !nearlyEqual(gridMaxDb, previousGridMaxDb)
           || !nearlyEqual(gridStepDb, previousGridStepDb);
}

void AnalyzerComponent::rebuildEnabledTraces() {
    viewState.enabledTraces.clear();
    for (size_t slotIndex = 0; slotIndex < signalSlots.size(); ++slotIndex) {
        const auto &slot = signalSlots[slotIndex];
        if (slot.configuration.enabled && slot.visible)
            viewState.enabledTraces.push_back(Analyzer::traceKindForSlot(slotIndex));
    }
}

void AnalyzerComponent::refreshStaticViewModelIfNeeded() {
    StaticViewStateKey nextKey;
    nextKey.bounds = getLocalBounds();
    nextKey.bandCount = renderData.bandInfo.size();
    nextKey.gridMinDb = gridMinDb;
    nextKey.gridMaxDb = gridMaxDb;
    nextKey.gridStepDb = gridStepDb;
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
    viewModel.updateStaticLayout(renderData, viewState, gridMinDb, gridMaxDb, gridStepDb, getLocalBounds().toFloat());
}

void AnalyzerComponent::rebuildDynamicViewModel() {
    viewModel.updateTraceVisuals(renderData, viewState, signalSlots, signalSlotOrder, gridMinDb, gridMaxDb);
}

void AnalyzerComponent::updateHoverState() {
    viewModel.updateHover(renderData, viewState, signalSlotOrder, meterSettings, gridMinDb,
                          getLocalBounds().toFloat(), hoverPosition);
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
    layerGraphics.fillAll(theme.analyzerBackground);

    const auto plotBounds = viewModel.getPlotBounds();
    layerGraphics.setColour(theme.plotBackground);
    layerGraphics.fillRoundedRectangle(plotBounds.expanded(6.0f, 6.0f), 10.0f);
    drawGrid(layerGraphics);
    staticLayerDirty = false;
}

void AnalyzerComponent::repaintHoverDelta(const std::optional<AnalyzerHoverInfo> &previousHoverInfo) {
    auto dirtyBounds = getHoverDirtyBounds(previousHoverInfo);
    dirtyBounds = dirtyBounds.getUnion(getHoverDirtyBounds(viewModel.getHoverInfo()));

    if (!dirtyBounds.isEmpty())
        repaint(dirtyBounds);
}

juce::Rectangle<int> AnalyzerComponent::getHoverDirtyBounds(const std::optional<AnalyzerHoverInfo> &hoverInfo) const {
    if (!hoverInfo.has_value())
        return {};

    auto dirtyBounds = hoverInfo->bounds.getSmallestIntegerContainer().expanded(2);
    if (const auto bandBounds = viewModel.getBandBounds(hoverInfo->bandIndex); bandBounds.has_value())
        dirtyBounds = dirtyBounds.getUnion(bandBounds->getSmallestIntegerContainer().expanded(2));

    return dirtyBounds;
}

void AnalyzerComponent::setIdlePolling(const bool shouldUseIdlePolling) {
    if (isIdlePolling == shouldUseIdlePolling)
        return;

    isIdlePolling = shouldUseIdlePolling;
    startTimer(isIdlePolling ? Analyzer::Constants::idleMeterPollIntervalMs
                             : Analyzer::Constants::meterPollIntervalMs);
}

void AnalyzerComponent::syncFreezeSnapshotIfNeeded() {
    const auto isFrozen = dataSource.isFrozen();
    if (isFrozen && !wasFrozen) {
        renderData = lastPaintedRenderData;
        refreshUiSnapshot();
        rebuildEnabledTraces();
        refreshStaticViewModelIfNeeded();
        rebuildDynamicViewModel();
        updateHoverState();
    }

    wasFrozen = isFrozen;
}

void AnalyzerComponent::timerCallback() {
    syncFreezeSnapshotIfNeeded();

    const auto currentPollTimeMs = juce::Time::getMillisecondCounterHiRes();
    // The meter uses real elapsed time so decay stays correct even if the timer jitters a bit
    const auto dtSeconds = static_cast<float>((currentPollTimeMs - lastPollTimeMs) * 0.001);
    lastPollTimeMs = currentPollTimeMs;
    const auto uiSnapshotChanged = refreshUiSnapshot();
    const auto nextBandInfo = dataSource.getBandInfo();
    const auto bandLayoutChanged = bandInfo != nextBandInfo;

    if (!dataSource.isFrozen()) {
        bandInfo = nextBandInfo;

        const auto shouldAdvanceDisplay = dataSource.hasRecentSignal() || !displayMeter.isSettledAtFloor(gridMinDb);
        setIdlePolling(!shouldAdvanceDisplay);

        if (!shouldAdvanceDisplay && !uiSnapshotChanged && !bandLayoutChanged)
            return;

        if (shouldAdvanceDisplay || bandLayoutChanged) {
            rawTraces = dataSource.getRawTraces();
            // Raw DSP measurements become render-ready RMS, peak, and hold values here
            displayMeter.tick(bandInfo, rawTraces, meterSettings, gridMinDb, dtSeconds);
            renderData = displayMeter.getRenderData();
        }
    } else {
        setIdlePolling(false);
        bandInfo = nextBandInfo;
    }

    rebuildEnabledTraces();
    refreshStaticViewModelIfNeeded();
    rebuildDynamicViewModel();
    updateHoverState();
    repaint();
}
