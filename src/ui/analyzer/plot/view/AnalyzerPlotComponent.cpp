#include "ui/analyzer/plot/view/AnalyzerPlotComponent.h"

#include <cmath>

AnalyzerPlotComponent::AnalyzerPlotComponent(AnalyzerRawTraceSource &rawTraceSourceToUse,
                                             const Ui::Theme &themeToUse)
    : theme(themeToUse),
      displayWorker(rawTraceSourceToUse, *this),
      traceBatchBuilder(themeToUse),
      globalHoldBatchBuilder(themeToUse),
      hoverHighlightBatchBuilder(themeToUse) {
    setOpaque(true);
    displayWorker.start();
}

AnalyzerPlotComponent::~AnalyzerPlotComponent() {
    cancelPendingUpdate();
    displayWorker.stop();
}

void AnalyzerPlotComponent::setListener(Listener *listenerToUse) {
    listener = listenerToUse;
}

void AnalyzerPlotComponent::setLayout(const AnalyzerSectionLayout &newLayout) {
    layout = newLayout;
    if (hoveredBandIndex.has_value() && *hoveredBandIndex >= layout->plotVisibleBands.size())
        hoveredBandIndex.reset();

    staticPlotLayerDirty = true;
    rebuildDynamicBatches();
    rebuildHoverHighlightBatches();
    repaint();
}

void AnalyzerPlotComponent::setUiSnapshot(const Ui::AnalyzerUiSnapshot &snapshot) {
    const auto previousControlState = hasUiSnapshot ? std::optional(makeDisplayControlState(uiSnapshot)) : std::nullopt;
    const auto snapshotChanged = !hasUiSnapshot || uiSnapshot != snapshot;
    uiSnapshot = snapshot;
    hasUiSnapshot = true;

    const auto nextControlState = makeDisplayControlState(uiSnapshot);
    if (!previousControlState.has_value() || *previousControlState != nextControlState)
        displayWorker.setControlState(nextControlState);

    if (!snapshotChanged)
        return;

    rebuildDynamicBatches();
    rebuildHoverHighlightBatches();
    repaint();
}

void AnalyzerPlotComponent::setHoveredBandIndex(const std::optional<size_t> &newHoveredBandIndex) {
    if (hoveredBandIndex == newHoveredBandIndex)
        return;

    auto repaintBounds = getHoverRepaintBounds(hoveredBandIndex);
    repaintBounds = repaintBounds.getUnion(getHoverRepaintBounds(newHoveredBandIndex));
    hoveredBandIndex = newHoveredBandIndex;
    rebuildHoverHighlightBatches();

    if (!repaintBounds.isEmpty())
        repaint(repaintBounds);
}

void AnalyzerPlotComponent::paint(juce::Graphics &g) {
    ensureStaticPlotLayer();
    g.drawImageAt(staticPlotLayer, 0, 0);
    drawBars(g);
    drawGlobalHold(g);
    drawHoverHighlight(g);
}

void AnalyzerPlotComponent::resized() {
    staticPlotLayerDirty = true;
    repaint();
}

void AnalyzerPlotComponent::mouseMove(const juce::MouseEvent &event) {
    notifyHoverChanged(event.position);
}

void AnalyzerPlotComponent::mouseDrag(const juce::MouseEvent &event) {
    notifyHoverChanged(event.position);
}

void AnalyzerPlotComponent::mouseExit(const juce::MouseEvent &) {
    notifyHoverChanged(std::nullopt);
}

void AnalyzerPlotComponent::drawGrid(juce::Graphics &g) const {
    if (!layout.has_value())
        return;

    for (const auto &gridLine: layout->gridLines) {
        g.setColour(theme.gridLine.withMultipliedAlpha(0.9f));
        g.drawHorizontalLine(static_cast<int>(std::round(gridLine.plotLocalY)), 0.0f, layout->plotLocalBounds.getRight());
    }

    for (const auto &frequencyMarker: layout->frequencyMarkers) {
        g.setColour(theme.gridLine.withMultipliedAlpha(0.62f));
        g.drawVerticalLine(static_cast<int>(std::round(frequencyMarker.plotLocalX)), 0.0f, layout->plotLocalBounds.getBottom());
    }
}

void AnalyzerPlotComponent::drawBars(juce::Graphics &g) const {
    for (const auto &batch: traceBatchBuilder.getBatches()) {
        if (batch.rectangles.isEmpty())
            continue;

        g.setColour(batch.colour);
        g.fillRectList(batch.rectangles);
    }
}

void AnalyzerPlotComponent::drawGlobalHold(juce::Graphics &g) const {
    for (const auto &batch: globalHoldBatchBuilder.getBatches()) {
        if (batch.rectangles.isEmpty())
            continue;

        g.setColour(batch.colour);
        g.fillRectList(batch.rectangles);
    }
}

void AnalyzerPlotComponent::drawHoverHighlight(juce::Graphics &g) const {
    for (const auto &batch: hoverHighlightBatchBuilder.getBatches()) {
        if (batch.rectangles.isEmpty())
            continue;

        g.setColour(batch.colour);
        g.fillRectList(batch.rectangles);
    }
}

void AnalyzerPlotComponent::ensureStaticPlotLayer() {
    if (!staticPlotLayerDirty)
        return;

    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        staticPlotLayer = {};
        staticPlotLayerDirty = false;
        return;
    }

    staticPlotLayer = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics layerGraphics(staticPlotLayer);
    const auto localBounds = bounds.toFloat();
    const auto &plotMetrics = theme.metrics.analyzerPlot;
    juce::ColourGradient plotGradient(theme.plotBackground.brighter(plotMetrics.gradientTopBrightness),
                                      localBounds.getCentreX(),
                                      localBounds.getY(),
                                      theme.plotBackground.darker(plotMetrics.gradientBottomDarkness),
                                      localBounds.getCentreX(),
                                      localBounds.getBottom(),
                                      false);
    plotGradient.addColour(plotMetrics.gradientMidPoint, theme.plotBackground);
    layerGraphics.setGradientFill(plotGradient);
    layerGraphics.fillRect(localBounds);

    drawGrid(layerGraphics);
    staticPlotLayerDirty = false;
}

void AnalyzerPlotComponent::rebuildDynamicBatches() {
    traceBatchBuilder.reset();
    globalHoldBatchBuilder.reset();

    if (!layout.has_value() || !hasUiSnapshot)
        return;

    traceBatchBuilder.buildTraceBatches(displayFrame,
                                        layout->plotVisibleBands,
                                        uiSnapshot.signalSlots,
                                        uiSnapshot.slotOrder,
                                        uiSnapshot.meterSettings,
                                        layout->gridMinDb,
                                        layout->gridMaxDb,
                                        layout->plotLocalBounds,
                                        layout->plotLocalBounds);
    globalHoldBatchBuilder.buildGlobalHoldBatches(displayFrame,
                                                  layout->plotVisibleBands,
                                                  uiSnapshot.signalSlots,
                                                  uiSnapshot.meterSettings,
                                                  layout->gridMinDb,
                                                  layout->gridMaxDb,
                                                  layout->plotLocalBounds,
                                                  layout->plotLocalBounds);
}

void AnalyzerPlotComponent::rebuildHoverHighlightBatches() {
    hoverHighlightBatchBuilder.reset();

    if (!layout.has_value() || !hasUiSnapshot || !hoveredBandIndex.has_value())
        return;

    if (*hoveredBandIndex >= layout->plotVisibleBands.size())
        return;

    hoverHighlightBatchBuilder.buildHoveredBarBatches(displayFrame,
                                                      layout->plotVisibleBands,
                                                      uiSnapshot.signalSlots,
                                                      uiSnapshot.slotOrder,
                                                      uiSnapshot.meterSettings,
                                                      *hoveredBandIndex,
                                                      layout->gridMinDb,
                                                      layout->gridMaxDb,
                                                      layout->plotLocalBounds,
                                                      layout->plotVisibleBands[*hoveredBandIndex].hitBounds);
}

void AnalyzerPlotComponent::notifyHoverChanged(const std::optional<juce::Point<float>> &plotLocalPosition) const {
    if (listener != nullptr)
        listener->analyzerPlotHoverChanged(plotLocalPosition);
}

AnalyzerDisplayControlState AnalyzerPlotComponent::makeDisplayControlState(const Ui::AnalyzerUiSnapshot &snapshot) const {
    AnalyzerDisplayControlState controlState;
    controlState.meterSettings = snapshot.meterSettings;
    controlState.floorDb = snapshot.gridMinDb;
    controlState.globalFrozen = snapshot.frozen;

    for (size_t slotIndex = 0; slotIndex < snapshot.signalSlots.size(); ++slotIndex) {
        const auto &slot = snapshot.signalSlots[slotIndex];
        controlState.slotFrozen[slotIndex] = slot.configuration.enabled && slot.frozen;
        controlState.slotContributing[slotIndex] = slot.configuration.enabled && slot.visible;
    }

    return controlState;
}

juce::Rectangle<int> AnalyzerPlotComponent::getHoverRepaintBounds(const std::optional<size_t> &bandIndex) const {
    if (!layout.has_value() || !bandIndex.has_value() || *bandIndex >= layout->plotVisibleBands.size())
        return {};

    return layout->plotVisibleBands[*bandIndex].hitBounds.getSmallestIntegerContainer().expanded(2);
}

void AnalyzerPlotComponent::handleAsyncUpdate() {
    bool hasUpdate = false;
    const auto *nextFrame = displayWorker.readLatestFrame(hasUpdate);
    if (nextFrame == nullptr || (!hasUpdate && nextFrame->revision == lastConsumedRevision))
        return;

    displayFrame = nextFrame;
    lastConsumedRevision = nextFrame->revision;

    if (nextFrame->bandInfo != nullptr && nextFrame->bandInfo != currentBandInfo) {
        currentBandInfo = nextFrame->bandInfo;
        if (listener != nullptr)
            listener->analyzerPlotBandInfoChanged(currentBandInfo);
    }

    rebuildDynamicBatches();
    rebuildHoverHighlightBatches();
    repaint();
}

void AnalyzerPlotComponent::analyzerDisplayFramePublished() {
    triggerAsyncUpdate();
}
