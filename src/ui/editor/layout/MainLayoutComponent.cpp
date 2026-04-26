#include "MainLayoutComponent.h"

MainLayoutComponent::MainLayoutComponent(AnalyzerRawTraceSource& rawTraceSourceToUse,
                                         AnalyzerUiSnapshotSource& snapshotSourceToUse,
                                         PresetUiSnapshotSource& presetUiSnapshotSourceToUse,
                                         AnalyzerSettingsActions& settingsActionsToUse,
                                         PresetActions& presetActionsToUse,
                                         const Ui::Theme& themeToUse)
    : theme(themeToUse),
      presetHeaderComponent(presetUiSnapshotSourceToUse, presetActionsToUse, themeToUse),
      analyzerSectionComponent(rawTraceSourceToUse,
                               snapshotSourceToUse,
                               themeToUse),
      signalRackComponent(snapshotSourceToUse, settingsActionsToUse, themeToUse),
      meterControlsComponent(snapshotSourceToUse, settingsActionsToUse, themeToUse),
      verticalSectionDivider(themeToUse, SectionDividerComponent::Orientation::vertical),
      horizontalSectionDivider(themeToUse, SectionDividerComponent::Orientation::horizontal) {
    addAndMakeVisible(analyzerSectionComponent);
    addAndMakeVisible(presetHeaderComponent);
    addAndMakeVisible(signalRackComponent);
    addAndMakeVisible(verticalSectionDivider);
    addAndMakeVisible(horizontalSectionDivider);
    addAndMakeVisible(meterControlsComponent);
}

void MainLayoutComponent::resized() {
    const auto layout = computeLayout();

    analyzerSectionComponent.setBounds(layout.analyzerBounds);
    analyzerSectionComponent.setTopReservedHeight(theme.metrics.presetHeader.topInset
                                                  + theme.metrics.presetHeader.height
                                                  + theme.metrics.presetHeader.plotGap);
    presetHeaderComponent.setBounds(layout.presetHeaderBounds);
    signalRackComponent.setBounds(layout.rackBounds);
    meterControlsComponent.setBounds(layout.actionsBounds);
    verticalSectionDivider.setBounds(layout.verticalDividerBounds);
    horizontalSectionDivider.setBounds(layout.horizontalDividerBounds);
}

MainLayoutComponent::Layout MainLayoutComponent::computeLayout() const {
    const auto& metrics = theme.metrics.panel;
    const auto dividerThickness = theme.metrics.sectionDivider.thickness;
    const auto woodSideInset = theme.metrics.background.woodSideInset;

    Layout layout;
    const auto bounds = getLocalBounds().reduced(woodSideInset, 0);

    auto topAndGapBounds = bounds;
    layout.rackBounds = topAndGapBounds.removeFromBottom(metrics.rackHeight);
    topAndGapBounds.removeFromBottom(metrics.analyzerToRackGap);
    auto topSectionBounds = topAndGapBounds;
    layout.horizontalDividerBounds = juce::Rectangle<int>(
        bounds.getX(),
        layout.rackBounds.getY() - dividerThickness,
        bounds.getWidth(),
        dividerThickness);
    topSectionBounds.setBottom(layout.horizontalDividerBounds.getY());

    const auto rawActionsBounds = topSectionBounds.removeFromRight(metrics.sideStripWidth);
    const auto verticalGapBounds = topSectionBounds.removeFromRight(metrics.analyzerToSideStripGap);
    const auto verticalDividerTop = topSectionBounds.getY();
    const auto verticalDividerBottom = layout.horizontalDividerBounds.getY();
    const auto verticalDividerHeight = juce::jmax(0, verticalDividerBottom - verticalDividerTop);
    layout.verticalDividerBounds = juce::Rectangle<int>(dividerThickness, verticalDividerHeight)
                                       .withCentre({verticalGapBounds.getCentreX(), verticalDividerTop + verticalDividerHeight / 2});
    layout.analyzerBounds = topSectionBounds.withRight(layout.verticalDividerBounds.getX());
    layout.actionsBounds = rawActionsBounds.withX(layout.verticalDividerBounds.getRight());

    const auto& headerMetrics = theme.metrics.presetHeader;
    const auto targetPlotBounds = layout.analyzerBounds.reduced(theme.metrics.analyzerSection.plotInset);
    layout.presetHeaderBounds = juce::Rectangle<int>(targetPlotBounds.getX(),
                                                     layout.analyzerBounds.getY() + headerMetrics.topInset,
                                                     targetPlotBounds.getWidth(),
                                                     headerMetrics.height);
    return layout;
}
