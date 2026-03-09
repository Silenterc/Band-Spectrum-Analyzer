#include "AnalyzerComponent.h"

AnalyzerComponent::AnalyzerComponent(AnalyzerDataSource &source, const Ui::Theme &themeToUse)
    : dataSource(source), theme(themeToUse) {
    snapshot = dataSource.getSnapshot();
    rebuildViewModel();
    startTimerHz(30);
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

            const auto topColour = bar.isHovered ? theme.hoveredBarTop : theme.barTop;
            const auto bottomColour = bar.isHovered ? theme.hoveredBarBottom : theme.barBottom;

            if (bar.rmsDb > viewModel.getGridMinDb()) {
                g.setGradientFill(juce::ColourGradient(topColour, bar.rmsBounds.getCentreX(), bar.rmsBounds.getY(),
                                                       bottomColour, bar.rmsBounds.getCentreX(), bar.rmsBounds.getBottom(), false));
                g.fillRoundedRectangle(bar.rmsBounds, 2.0f);
            }

            if (bar.peakDb > viewModel.getGridMinDb()) {
                g.setColour(bar.isHovered ? theme.hoveredBarTop : theme.tooltipText);
                g.fillRect(bar.rmsBounds.getX(), bar.peakY - 1.0f, bar.rmsBounds.getWidth(), 2.0f);
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

    const auto tooltipText = hoverInfo.levelText + "\n"
                             + hoverInfo.frequencyText + "\n"
                             + hoverInfo.noteText;

    g.setColour(theme.tooltipText);
    g.setFont(12.0f);
    g.drawFittedText(tooltipText, hoverInfo.bounds.toNearestInt().reduced(10, 8), juce::Justification::centredLeft, 3);
}

void AnalyzerComponent::rebuildViewModel() {
    viewModel.update(snapshot, viewState, dataSource.getGridMinDb(), dataSource.getGridMaxDb(), dataSource.getGridStepDb(),
                     getLocalBounds().toFloat(), hoverPosition);
}

void AnalyzerComponent::timerCallback() {
    snapshot = dataSource.getSnapshot();
    rebuildViewModel();
    repaint();
}
