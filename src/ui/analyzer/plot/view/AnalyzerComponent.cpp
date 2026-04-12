#include "ui/analyzer/plot/view/AnalyzerComponent.h"

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

AnalyzerComponent::AnalyzerComponent(AnalyzerRawTraceSource &rawTraceSourceToUse,
                                     AnalyzerUiSnapshotSource &snapshotSourceToUse,
                                     const Ui::Theme &themeToUse)
    : rawTraceSource(rawTraceSourceToUse),
      snapshotSource(snapshotSourceToUse),
      theme(themeToUse),
      displayWorker(rawTraceSourceToUse, *this),
      viewModel(themeToUse),
      hoverOverlay(themeToUse),
      traceBatchBuilder(themeToUse),
      globalHoldBatchBuilder(themeToUse) {
    setOpaque(false);

    uiSnapshot = snapshotSource.getAnalyzerUiSnapshot();
    snapshotSource.addAnalyzerUiSnapshotListener(*this);
    juce::ignoreUnused(rebuildPresentationModel());
    displayWorker.setControlState(makeDisplayControlState(uiSnapshot));
    displayWorker.start();
}

AnalyzerComponent::~AnalyzerComponent() {
    cancelPendingUpdate();
    displayWorker.stop();
    snapshotSource.removeAnalyzerUiSnapshotListener(*this);
}

void AnalyzerComponent::paint(juce::Graphics &g) {
    ensureStaticLayer();
    g.drawImageAt(staticLayer, 0, 0);
    drawBars(g);
    drawGlobalHold(g);
    drawHover(g);
}

void AnalyzerComponent::resized() {
    staticLayerDirty = true;
    juce::ignoreUnused(rebuildPresentationModel());
    hoverUpdatePending = false;
    repaint();
}

void AnalyzerComponent::mouseMove(const juce::MouseEvent &event) {
    hoverPosition = event.position;
    requestHoverUpdate();
}

void AnalyzerComponent::mouseDrag(const juce::MouseEvent &event) {
    hoverPosition = event.position;
    requestHoverUpdate();
}

void AnalyzerComponent::mouseExit(const juce::MouseEvent &) {
    hoverPosition.reset();
    requestHoverUpdate();
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
    for (const auto &batch: traceBatchBuilder.getBatches()) {
        if (batch.rectangles.isEmpty())
            continue;

        g.setColour(batch.colour);
        g.fillRectList(batch.rectangles);
    }
}

void AnalyzerComponent::drawGlobalHold(juce::Graphics &g) const {
    for (const auto &batch: globalHoldBatchBuilder.getBatches()) {
        if (batch.rectangles.isEmpty())
            continue;

        g.setColour(batch.colour);
        g.fillRectList(batch.rectangles);
    }
}

void AnalyzerComponent::drawHover(juce::Graphics &g) const {
    hoverOverlay.draw(g);
}

juce::Rectangle<int> AnalyzerComponent::rebuildPresentationModel() {
    refreshStaticViewModelIfNeeded();
    rebuildDynamicBatches();
    return updateHoverState();
}

void AnalyzerComponent::refreshStaticViewModelIfNeeded() {
    currentBandInfo = getCurrentBandInfo();
    if (currentBandInfo == nullptr)
        return;

    StaticViewStateKey nextKey;
    nextKey.bounds = getLocalBounds();
    nextKey.bandCount = currentBandInfo->size();
    nextKey.gridMinDb = uiSnapshot.gridMinDb;
    nextKey.gridMaxDb = uiSnapshot.gridMaxDb;
    nextKey.gridStepDb = uiSnapshot.gridStepDb;
    nextKey.useCustomFrequencyRange = viewState.useCustomFrequencyRange;
    nextKey.visibleMinFrequencyHz = viewState.visibleMinFrequencyHz;
    nextKey.visibleMaxFrequencyHz = viewState.visibleMaxFrequencyHz;

    if (!currentBandInfo->empty()) {
        nextKey.minBandFrequencyHz = currentBandInfo->front().lowHz;
        nextKey.maxBandFrequencyHz = currentBandInfo->back().highHz;
    }

    if (staticViewStateKey.has_value() && *staticViewStateKey == nextKey)
        return;

    staticViewStateKey = nextKey;
    staticLayerDirty = true;
    viewModel.updateStaticLayout(*currentBandInfo,
                                 viewState,
                                 uiSnapshot.gridMinDb,
                                 uiSnapshot.gridMaxDb,
                                 uiSnapshot.gridStepDb,
                                 getLocalBounds().toFloat());
}

void AnalyzerComponent::rebuildDynamicBatches() {
    const auto plotBounds = viewModel.getPlotBounds();
    ++hoverVisualRevision;
    traceBatchBuilder.buildTraceBatches(displayFrame,
                                        viewModel.getVisibleBands(),
                                        uiSnapshot.signalSlots,
                                        uiSnapshot.slotOrder,
                                        uiSnapshot.meterSettings,
                                        uiSnapshot.gridMinDb,
                                        uiSnapshot.gridMaxDb,
                                        plotBounds,
                                        plotBounds);
    globalHoldBatchBuilder.buildGlobalHoldBatches(displayFrame,
                                                  viewModel.getVisibleBands(),
                                                  uiSnapshot.signalSlots,
                                                  uiSnapshot.meterSettings,
                                                  uiSnapshot.gridMinDb,
                                                  uiSnapshot.gridMaxDb,
                                                  plotBounds,
                                                  plotBounds);
}

juce::Rectangle<int> AnalyzerComponent::updateHoverState() {
    viewModel.updateHover(uiSnapshot.gridMinDb, uiSnapshot.gridMaxDb, getLocalBounds().toFloat(), hoverPosition);

    std::optional<juce::Rectangle<float>> hoveredBandBounds;
    if (const auto &hoverInfo = viewModel.getHoverInfo(); hoverInfo.has_value()) {
        const auto &visibleBands = viewModel.getVisibleBands();
        if (hoverInfo->bandIndex < visibleBands.size())
            hoveredBandBounds = visibleBands[hoverInfo->bandIndex].drawBounds;
    }

    // The message thread only consumes immutable display frames; semantic analyzer evolution stays on the worker.
    return hoverOverlay.updateState(viewModel.getHoverInfo(),
                                    displayFrame,
                                    viewModel.getVisibleBands(),
                                    uiSnapshot.signalSlots,
                                    uiSnapshot.slotOrder,
                                    hoverVisualRevision,
                                    uiSnapshot.meterSettings,
                                    uiSnapshot.gridMinDb,
                                    uiSnapshot.gridMaxDb,
                                    viewModel.getPlotBounds(),
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

void AnalyzerComponent::repaintAnalyzer(const juce::Rectangle<int> &additionalBounds, const bool includeDynamicBounds) {
    if (staticLayerDirty && includeDynamicBounds) {
        repaint();
        return;
    }

    auto repaintBounds = additionalBounds;
    if (includeDynamicBounds)
        repaintBounds = repaintBounds.getUnion(getDynamicRepaintBounds());

    if (!repaintBounds.isEmpty())
        repaint(repaintBounds);
}

juce::Rectangle<int> AnalyzerComponent::getDynamicRepaintBounds() const {
    return viewModel.getPlotBounds().expanded(2.0f).getSmallestIntegerContainer();
}

std::shared_ptr<const std::vector<Analyzer::BandInfo>> AnalyzerComponent::getCurrentBandInfo() const {
    if (displayFrame != nullptr && displayFrame->bandInfo != nullptr)
        return displayFrame->bandInfo;

    return rawTraceSource.getBandInfo();
}

AnalyzerDisplayControlState AnalyzerComponent::makeDisplayControlState(const Ui::AnalyzerUiSnapshot &snapshot) const {
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

void AnalyzerComponent::handleAsyncUpdate() {
    auto didUpdatePresentation = false;
    bool hasUpdate = false;
    const auto *nextFrame = displayWorker.readLatestFrame(hasUpdate);
    if (nextFrame != nullptr && (hasUpdate || nextFrame->revision != lastConsumedRevision)) {
        displayFrame = nextFrame;
        lastConsumedRevision = nextFrame->revision;
        const auto hoverDirtyBounds = rebuildPresentationModel();
        hoverUpdatePending = false;
        didUpdatePresentation = true;
        repaintAnalyzer(hoverDirtyBounds, true);
    }

    if (hoverUpdatePending) {
        const auto hoverDirtyBounds = updateHoverState();
        hoverUpdatePending = false;
        didUpdatePresentation = true;
        repaintAnalyzer(hoverDirtyBounds, false);
    }

    juce::ignoreUnused(didUpdatePresentation);
}

void AnalyzerComponent::analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) {
    if (uiSnapshot == snapshot)
        return;

    const auto previousControlState = makeDisplayControlState(uiSnapshot);
    uiSnapshot = snapshot;

    if (previousControlState != makeDisplayControlState(uiSnapshot))
        displayWorker.setControlState(makeDisplayControlState(uiSnapshot));

    const auto hoverDirtyBounds = rebuildPresentationModel();
    hoverUpdatePending = false;
    repaintAnalyzer(hoverDirtyBounds, true);
}

void AnalyzerComponent::analyzerDisplayFramePublished() {
    triggerAsyncUpdate();
}

void AnalyzerComponent::requestHoverUpdate() {
    hoverUpdatePending = true;
    triggerAsyncUpdate();
}
