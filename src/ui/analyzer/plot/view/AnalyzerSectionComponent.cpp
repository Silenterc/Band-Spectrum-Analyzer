#include "ui/analyzer/plot/view/AnalyzerSectionComponent.h"

#include <cmath>

#include "ui/theme/UiRasterAssets.h"

namespace {
    bool nearlyEqual(const float lhs, const float rhs) {
        return std::abs(lhs - rhs) <= 0.0001f;
    }
}

AnalyzerSectionComponent::AnalyzerSectionComponent(AnalyzerRawTraceSource &rawTraceSourceToUse,
                                                   AnalyzerUiSnapshotSource &snapshotSourceToUse,
                                                   PresetUiSnapshotSource& presetUiSnapshotSourceToUse,
                                                   PresetActions& presetActionsToUse,
                                                   const Ui::Theme &themeToUse)
    : rawTraceSource(rawTraceSourceToUse),
      snapshotSource(snapshotSourceToUse),
      theme(themeToUse),
      presetHeaderComponent(presetUiSnapshotSourceToUse, presetActionsToUse, themeToUse),
      analyzerPlotComponent(rawTraceSourceToUse, themeToUse),
      viewModel(themeToUse),
      hoverTooltipRenderer(themeToUse) {
    setOpaque(true);
    addAndMakeVisible(presetHeaderComponent);
    analyzerPlotComponent.setListener(this);
    addAndMakeVisible(analyzerPlotComponent);

    uiSnapshot = snapshotSource.getAnalyzerUiSnapshot();
    currentBandInfo = rawTraceSource.getBandInfo();
    analyzerPlotComponent.setUiSnapshot(uiSnapshot);
    snapshotSource.addAnalyzerUiSnapshotListener(*this);
    rebuildLayout();
}

AnalyzerSectionComponent::~AnalyzerSectionComponent() {
    snapshotSource.removeAnalyzerUiSnapshotListener(*this);
}

void AnalyzerSectionComponent::paint(juce::Graphics &g) {
    g.drawImageAt(cachedBackground, 0, 0);
}

void AnalyzerSectionComponent::paintOverChildren(juce::Graphics &g) {
    hoverTooltipRenderer.draw(g);
}

void AnalyzerSectionComponent::resized() {
    rebuildLayout();
}

void AnalyzerSectionComponent::drawAxisLabels(juce::Graphics &g) const {
    const auto &layout = viewModel.getLayout();
    const auto &plotMetrics = theme.metrics.analyzerPlot;
    const auto gridLabelY = [&](const float sectionY) {
        return juce::roundToInt(sectionY - static_cast<float>(plotMetrics.gridLabelHeight) * 0.5f);
    };
    const auto frequencyLabelX = [&](const float sectionX) {
        return juce::roundToInt(sectionX - plotMetrics.frequencyLabelXHalfSpan);
    };
    const auto frequencyLabelY = juce::roundToInt(layout.plotBounds.getBottom() + plotMetrics.frequencyLabelYOffset);

    g.setColour(theme.axisText);
    g.setFont(plotMetrics.gridLabelFontHeight);
    for (const auto &gridLine: layout.gridLines) {
        g.drawText(gridLine.label,
                   juce::roundToInt(layout.displayBounds.getX()),
                   gridLabelY(gridLine.sectionY),
                   plotMetrics.gridLabelWidth,
                   plotMetrics.gridLabelHeight,
                   juce::Justification::centredRight);
    }

    for (const auto &frequencyMarker: layout.frequencyMarkers) {
        g.drawText(frequencyMarker.label,
                   frequencyLabelX(frequencyMarker.sectionX),
                   frequencyLabelY,
                   plotMetrics.frequencyLabelWidth,
                   plotMetrics.frequencyLabelHeight,
                   juce::Justification::centred);
    }
}

void AnalyzerSectionComponent::drawTopCornerScrews(juce::Graphics &g, const juce::Rectangle<int> &bounds) const {
    const auto &screw = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::screw);
    const auto rasterScale = theme.metrics.assets.rasterScale;
    const auto screwPadding = theme.metrics.background.screwPadding;
    const auto topLeftBounds = Ui::getLogicalAssetBounds(screw, rasterScale, {screwPadding, screwPadding});
    const auto topRightBounds = Ui::getLogicalAssetBounds(
        screw,
        rasterScale,
        {bounds.getWidth() - screwPadding - topLeftBounds.getWidth(), screwPadding});

    Ui::drawAssetWithin(g, screw, topLeftBounds);
    Ui::drawAssetWithin(g, screw, topRightBounds);
}

void AnalyzerSectionComponent::rebuildLayout() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        analyzerPlotComponent.setBounds({});
        cachedBackground = {};
        hoverPosition.reset();
        const auto dirtyBounds = hoverTooltipRenderer.updateState(std::nullopt);
        analyzerPlotComponent.setHoveredBandIndex(std::nullopt);
        if (!dirtyBounds.isEmpty())
            repaint(dirtyBounds);
        return;
    }

    if (currentBandInfo == nullptr)
        currentBandInfo = rawTraceSource.getBandInfo();

    if (currentBandInfo == nullptr)
        return;

    viewState.useCustomFrequencyRange = uiSnapshot.useCustomFrequencyRange;
    viewState.visibleMinFrequencyHz = uiSnapshot.visibleMinFrequencyHz;
    viewState.visibleMaxFrequencyHz = uiSnapshot.visibleMaxFrequencyHz;

    const auto layout = computeLayout();
    viewModel.updateLayout(*currentBandInfo,
                           viewState,
                           uiSnapshot.gridMinDb,
                           uiSnapshot.gridMaxDb,
                           uiSnapshot.gridStepDb,
                           bounds.toFloat(),
                           layout.displayBounds.toFloat());
    presetHeaderComponent.setBounds(layout.headerBounds);
    analyzerPlotComponent.setBounds(viewModel.getLayout().plotBounds.getSmallestIntegerContainer());
    analyzerPlotComponent.setLayout(viewModel.getLayout());
    rebuildCachedBackground();
    updateHoverPresentation();
    repaint();
}

void AnalyzerSectionComponent::rebuildCachedBackground() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        cachedBackground = {};
        return;
    }

    cachedBackground = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics graphics(cachedBackground);
    const auto &backgroundImage = Ui::getAnalyzerRasterAsset(Ui::AnalyzerRasterAssetId::background2);
    graphics.drawImage(backgroundImage,
                       bounds.getX(),
                       bounds.getY(),
                       bounds.getWidth(),
                       bounds.getHeight(),
                       0,
                       0,
                       backgroundImage.getWidth(),
                       backgroundImage.getHeight());

    drawTopCornerScrews(graphics, bounds);
    drawAxisLabels(graphics);
}

void AnalyzerSectionComponent::updateHoverPresentation() {
    const auto hoverInfo = viewModel.buildHover(hoverPosition);
    const auto dirtyBounds = hoverTooltipRenderer.updateState(hoverInfo);
    analyzerPlotComponent.setHoveredBandIndex(hoverInfo.has_value() ? std::optional<size_t>(hoverInfo->bandIndex)
                                                                    : std::nullopt);

    if (!dirtyBounds.isEmpty())
        repaint(dirtyBounds);
}

AnalyzerSectionComponent::Layout AnalyzerSectionComponent::computeLayout() const {
    Layout layout;
    const auto originalTargetPlotBounds = getLocalBounds().reduced(theme.metrics.analyzerSection.plotInset);
    auto targetPlotBounds = originalTargetPlotBounds;
    const auto& headerMetrics = theme.metrics.presetHeader;
    layout.headerBounds = juce::Rectangle<int>(targetPlotBounds.getX(),
                                               headerMetrics.topInset,
                                               targetPlotBounds.getWidth(),
                                               headerMetrics.height);
    const auto plotTop = layout.headerBounds.getBottom() + headerMetrics.plotGap;
    const auto topReduction = juce::jmax(0, plotTop - originalTargetPlotBounds.getY());
    targetPlotBounds.removeFromTop(topReduction);
    const auto &plotMargins = theme.metrics.analyzerPlot;
    layout.displayBounds = juce::Rectangle<int>(juce::roundToInt(targetPlotBounds.getX() - plotMargins.plotMarginLeft),
                                                juce::roundToInt(targetPlotBounds.getY() - plotMargins.plotMarginTop),
                                                juce::roundToInt(targetPlotBounds.getWidth()
                                                                 + plotMargins.plotMarginLeft
                                                                 + plotMargins.plotMarginRight),
                                                juce::roundToInt(targetPlotBounds.getHeight()
                                                                 + plotMargins.plotMarginTop
                                                                 + plotMargins.plotMarginBottom));
    return layout;
}

void AnalyzerSectionComponent::analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) {
    if (uiSnapshot == snapshot)
        return;

    const auto layoutChanged = !nearlyEqual(uiSnapshot.gridMinDb, snapshot.gridMinDb)
                               || !nearlyEqual(uiSnapshot.gridMaxDb, snapshot.gridMaxDb)
                               || !nearlyEqual(uiSnapshot.gridStepDb, snapshot.gridStepDb)
                               || uiSnapshot.useCustomFrequencyRange != snapshot.useCustomFrequencyRange
                               || !nearlyEqual(uiSnapshot.visibleMinFrequencyHz, snapshot.visibleMinFrequencyHz)
                               || !nearlyEqual(uiSnapshot.visibleMaxFrequencyHz, snapshot.visibleMaxFrequencyHz);
    uiSnapshot = snapshot;
    analyzerPlotComponent.setUiSnapshot(uiSnapshot);

    if (layoutChanged)
        rebuildLayout();
}

void AnalyzerSectionComponent::analyzerPlotHoverChanged(
    const std::optional<juce::Point<float>> &plotLocalPosition) {
    hoverPosition = plotLocalPosition;
    updateHoverPresentation();
}

void AnalyzerSectionComponent::analyzerPlotBandInfoChanged(
    const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo) {
    if (bandInfo == nullptr || bandInfo == currentBandInfo)
        return;

    currentBandInfo = bandInfo;
    rebuildLayout();
}
