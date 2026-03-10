#include "AnalyzerComponent.h"
#include "../../dsp/AnalyzerConstants.h"

AnalyzerComponent::AnalyzerComponent(AnalyzerDataSource &source, const Ui::Theme &themeToUse)
    : dataSource(source), theme(themeToUse) {
    snapshot = dataSource.getSnapshot();
    // Prime the meter so the first paint already has render-ready values
    displayMeter.tick(snapshot, dataSource.getMeterSettings(), dataSource.getGridMinDb(), Analyzer::Constants::meterPollIntervalSeconds);
    renderData = displayMeter.getRenderData();
    lastPollTimeMs = juce::Time::getMillisecondCounterHiRes();
    rebuildViewModel();
    startTimer(Analyzer::Constants::meterPollIntervalMs);
}

void AnalyzerComponent::paint(juce::Graphics &g) {
    g.fillAll(theme.analyzerBackground);

    const auto plotBounds = viewModel.getPlotBounds();

    g.setColour(theme.plotBackground);
    g.fillRoundedRectangle(plotBounds.expanded(6.0f, 6.0f), 10.0f);

    drawGrid(g);
    drawBars(g);
    drawHoverInfo(g);
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

            const auto peakTopColour = bar.isHovered ? theme.hoveredBarTop : theme.barTop;
            const auto peakBottomColour = bar.isHovered ? theme.hoveredBarBottom : theme.barBottom;
            const auto rmsTopColour = bar.isHovered ? theme.hoveredRmsBarTop : theme.rmsBarTop;
            const auto rmsBottomColour = bar.isHovered ? theme.hoveredRmsBarBottom : theme.rmsBarBottom;

            if (bar.peakDb > viewModel.getGridMinDb()) {
                g.setGradientFill(juce::ColourGradient(peakTopColour, bar.peakBounds.getCentreX(), bar.peakBounds.getY(),
                                                       peakBottomColour, bar.peakBounds.getCentreX(), bar.peakBounds.getBottom(), false));
                g.fillRoundedRectangle(bar.peakBounds, 2.0f);
            }

            if (bar.rmsDb > viewModel.getGridMinDb()) {
                g.setGradientFill(juce::ColourGradient(rmsTopColour, bar.rmsBounds.getCentreX(), bar.rmsBounds.getY(),
                                                       rmsBottomColour, bar.rmsBounds.getCentreX(), bar.rmsBounds.getBottom(), false));
                g.fillRoundedRectangle(bar.rmsBounds, 2.0f);
            }

            const auto lineDb = bar.holdDb > viewModel.getGridMinDb() ? bar.holdDb : bar.peakDb;
            const auto lineY = bar.holdDb > viewModel.getGridMinDb() ? bar.holdY : bar.peakY;

            if (lineDb > viewModel.getGridMinDb()) {
                g.setColour(bar.isHovered ? theme.hoveredBarTop : theme.tooltipText);
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
    viewModel.update(renderData, viewState, dataSource.getMeterSettings(),
                     dataSource.getGridMinDb(), dataSource.getGridMaxDb(), dataSource.getGridStepDb(),
                     getLocalBounds().toFloat(), hoverPosition);
}

void AnalyzerComponent::timerCallback() {
    const auto currentPollTimeMs = juce::Time::getMillisecondCounterHiRes();
    // The meter uses real elapsed time so decay stays correct even if the timer jitters a bit
    const auto dtSeconds = static_cast<float>((currentPollTimeMs - lastPollTimeMs) * 0.001);
    lastPollTimeMs = currentPollTimeMs;

    snapshot = dataSource.getSnapshot();
    // Raw DSP measurements become render-ready RMS, peak, and hold values here
    displayMeter.tick(snapshot, dataSource.getMeterSettings(), dataSource.getGridMinDb(), dtSeconds);
    renderData = displayMeter.getRenderData();
    rebuildViewModel();
    repaint();
}
