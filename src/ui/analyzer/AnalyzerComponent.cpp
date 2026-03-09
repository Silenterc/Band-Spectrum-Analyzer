#include "AnalyzerComponent.h"

AnalyzerComponent::AnalyzerComponent(AnalyzerDataSource &source)
    : dataSource(source) {
    snapshot = dataSource.getSnapshot();
    rebuildViewModel();
    startTimerHz(30);
}

void AnalyzerComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colour::fromRGB(17, 18, 20));

    const auto plotBounds = viewModel.getPlotBounds();

    g.setColour(juce::Colour::fromRGB(28, 31, 35));
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

    g.setColour(juce::Colour::fromRGBA(255, 255, 255, 28));
    g.drawRoundedRectangle(plotBounds.expanded(1.0f), 8.0f, 1.0f);

    for (const auto &gridLine: viewModel.getGridLines()) {
        g.setColour(juce::Colour::fromRGBA(255, 255, 255, 20));
        g.drawHorizontalLine(static_cast<int>(std::round(gridLine.y)), plotBounds.getX(), plotBounds.getRight());

        g.setColour(juce::Colour::fromRGBA(255, 255, 255, 110));
        g.setFont(11.0f);
        g.drawText(gridLine.label, 0, static_cast<int>(gridLine.y - 7.0f), 48, 14, juce::Justification::centredRight);
    }

    for (const auto &frequencyMarker: viewModel.getFrequencyMarkers()) {
        g.setColour(juce::Colour::fromRGBA(255, 255, 255, 18));
        g.drawVerticalLine(static_cast<int>(std::round(frequencyMarker.x)), plotBounds.getY(), plotBounds.getBottom());

        g.setColour(juce::Colour::fromRGBA(255, 255, 255, 110));
        g.drawText(frequencyMarker.label, static_cast<int>(frequencyMarker.x - 18.0f),
                   static_cast<int>(plotBounds.getBottom() + 6.0f),
                   36, 16, juce::Justification::centred);
    }
}

void AnalyzerComponent::drawBars(juce::Graphics &g) const {
    for (const auto &bar: viewModel.getBars()) {
        if (bar.displayedDb <= viewModel.getGridMinDb())
            continue;

        const auto topColour = bar.isHovered ? juce::Colour::fromRGB(255, 214, 102) : juce::Colour::fromRGB(255, 157, 64);
        const auto bottomColour = bar.isHovered ? juce::Colour::fromRGB(255, 126, 69) : juce::Colour::fromRGB(255, 90, 95);

        g.setGradientFill(juce::ColourGradient(topColour, bar.bounds.getCentreX(), bar.bounds.getY(),
                                               bottomColour, bar.bounds.getCentreX(), bar.bounds.getBottom(), false));
        g.fillRoundedRectangle(bar.bounds, 2.0f);
    }
}

void AnalyzerComponent::drawHoverInfo(juce::Graphics &g) const {
    if (!viewModel.getHoverInfo().has_value())
        return;

    const auto &hoverInfo = *viewModel.getHoverInfo();

    g.setColour(juce::Colour::fromRGBA(10, 10, 12, 220));
    g.fillRoundedRectangle(hoverInfo.bounds, 8.0f);

    g.setColour(juce::Colour::fromRGBA(255, 255, 255, 36));
    g.drawRoundedRectangle(hoverInfo.bounds, 8.0f, 1.0f);

    const auto tooltipText = hoverInfo.levelText + "\n"
                             + hoverInfo.frequencyText + "\n"
                             + hoverInfo.noteText;

    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawFittedText(tooltipText, hoverInfo.bounds.toNearestInt().reduced(10, 8), juce::Justification::centredLeft, 3);
}

void AnalyzerComponent::rebuildViewModel() {
    viewModel.update(snapshot, dataSource.getGridMinDb(), dataSource.getGridMaxDb(), dataSource.getGridStepDb(),
                     getLocalBounds().toFloat(), hoverPosition);
}

void AnalyzerComponent::timerCallback() {
    snapshot = dataSource.getSnapshot();
    rebuildViewModel();
    repaint();
}
