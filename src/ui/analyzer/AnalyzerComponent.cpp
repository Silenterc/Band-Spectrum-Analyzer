#include "AnalyzerComponent.h"
#include "../../dsp/AnalyzerConstants.h"

AnalyzerComponent::AnalyzerComponent(AnalyzerDataSource &source, const Ui::Theme &themeToUse)
    : dataSource(source), theme(themeToUse) {
    bandInfo = dataSource.getBandInfo();
    rawTraces = dataSource.getRawTraces();
    // Prime the meter so the first paint already has render-ready values
    displayMeter.tick(bandInfo, rawTraces, dataSource.getMeterSettings(), dataSource.getGridMinDb(), Analyzer::Constants::meterPollIntervalSeconds);
    renderData = displayMeter.getRenderData();
    lastPaintedRenderData = renderData;
    wasFrozen = dataSource.isFrozen();
    lastPollTimeMs = juce::Time::getMillisecondCounterHiRes();
    rebuildViewModel();
    startTimer(Analyzer::Constants::meterPollIntervalMs);
}

void AnalyzerComponent::paint(juce::Graphics &g) {
    syncFreezeSnapshotIfNeeded();

    g.fillAll(theme.analyzerBackground);

    const auto plotBounds = viewModel.getPlotBounds();

    g.setColour(theme.plotBackground);
    g.fillRoundedRectangle(plotBounds.expanded(6.0f, 6.0f), 10.0f);

    drawGrid(g);
    drawBars(g);
    drawHoverInfo(g);

    lastPaintedRenderData = renderData;
}

void AnalyzerComponent::resized() {
    rebuildViewModel();
}

void AnalyzerComponent::mouseMove(const juce::MouseEvent &event) {
    hoverPosition = event.position;
    rebuildViewModel();
    repaint();
}

void AnalyzerComponent::mouseDrag(const juce::MouseEvent &event) {
    hoverPosition = event.position;
    rebuildViewModel();
    repaint();
}

void AnalyzerComponent::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hoverPosition.reset();
    rebuildViewModel();
    repaint();
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
    for (const auto &traceVisual: viewModel.getTraceVisuals()) {
        for (const auto &bar: traceVisual.bars) {
            if (bar.rmsDb <= viewModel.getGridMinDb() && bar.peakDb <= viewModel.getGridMinDb())
                continue;

            const auto peakColour = bar.isHovered ? traceVisual.colour.brighter(0.18f) : traceVisual.colour;
            const auto rmsColour = peakColour.withMultipliedAlpha(0.45f);

            if (bar.peakDb > viewModel.getGridMinDb()) {
                g.setColour(peakColour);
                g.fillRoundedRectangle(bar.peakBounds, 2.0f);
            }

            if (bar.rmsDb > viewModel.getGridMinDb()) {
                g.setColour(rmsColour);
                g.fillRoundedRectangle(bar.rmsBounds, 2.0f);
            }

            const auto lineDb = bar.holdDb > viewModel.getGridMinDb() ? bar.holdDb : bar.peakDb;
            const auto lineY = bar.holdDb > viewModel.getGridMinDb() ? bar.holdY : bar.peakY;

            if (lineDb > viewModel.getGridMinDb()) {
                g.setColour(bar.isHovered ? peakColour.brighter(0.1f) : peakColour.brighter(0.25f));
                g.fillRect(bar.peakBounds.getX(), lineY - 1.0f, bar.peakBounds.getWidth(), 2.0f);
            }
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

void AnalyzerComponent::rebuildViewModel() {
    viewState.enabledTraces.clear();
    const auto signalSlots = dataSource.getSignalSlots();
    const auto signalSlotOrder = dataSource.getSignalSlotOrder();
    for (size_t slotIndex = 0; slotIndex < signalSlots.size(); ++slotIndex) {
        const auto &slot = signalSlots[slotIndex];
        if (slot.configuration.enabled && slot.visible)
            viewState.enabledTraces.push_back(Analyzer::traceKindForSlot(slotIndex));
    }

    viewModel.update(renderData, viewState, signalSlots, signalSlotOrder, dataSource.getMeterSettings(),
                     dataSource.getGridMinDb(), dataSource.getGridMaxDb(), dataSource.getGridStepDb(),
                     getLocalBounds().toFloat(), hoverPosition);
}

void AnalyzerComponent::syncFreezeSnapshotIfNeeded() {
    const auto isFrozen = dataSource.isFrozen();
    if (isFrozen && !wasFrozen) {
        renderData = lastPaintedRenderData;
        rebuildViewModel();
    }

    wasFrozen = isFrozen;
}

void AnalyzerComponent::timerCallback() {
    syncFreezeSnapshotIfNeeded();

    const auto currentPollTimeMs = juce::Time::getMillisecondCounterHiRes();
    // The meter uses real elapsed time so decay stays correct even if the timer jitters a bit
    const auto dtSeconds = static_cast<float>((currentPollTimeMs - lastPollTimeMs) * 0.001);
    lastPollTimeMs = currentPollTimeMs;

    if (!dataSource.isFrozen()) {
        bandInfo = dataSource.getBandInfo();
        rawTraces = dataSource.getRawTraces();
        // Raw DSP measurements become render-ready RMS, peak, and hold values here
        displayMeter.tick(bandInfo, rawTraces, dataSource.getMeterSettings(), dataSource.getGridMinDb(), dtSeconds);
        renderData = displayMeter.getRenderData();
    }

    rebuildViewModel();
    repaint();
}
