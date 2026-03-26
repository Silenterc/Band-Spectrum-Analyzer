#include "MainLayoutComponent.h"

MainLayoutComponent::MainLayoutComponent(AnalyzerRenderSource& renderSourceToUse,
                                         AnalyzerUiSnapshotSource& snapshotSourceToUse,
                                         AnalyzerSettingsActions& settingsActionsToUse,
                                         const Ui::Theme& themeToUse)
    : theme(themeToUse),
      analyzerSectionComponent(renderSourceToUse, snapshotSourceToUse, themeToUse),
      signalRackComponent(snapshotSourceToUse, settingsActionsToUse, themeToUse),
      meterControlsComponent(snapshotSourceToUse, settingsActionsToUse, themeToUse),
      verticalSectionDivider(themeToUse, SectionDividerComponent::Orientation::vertical),
      horizontalSectionDivider(themeToUse, SectionDividerComponent::Orientation::horizontal) {
    addAndMakeVisible(analyzerSectionComponent);
    addAndMakeVisible(signalRackComponent);
    addAndMakeVisible(verticalSectionDivider);
    addAndMakeVisible(horizontalSectionDivider);
    addAndMakeVisible(meterControlsComponent);
}

MainLayoutComponent::~MainLayoutComponent() {
}

void MainLayoutComponent::paint(juce::Graphics& g) {
    juce::ignoreUnused(g);
}

void MainLayoutComponent::resized() {
    const auto layout = computeLayout();

    analyzerSectionComponent.setBounds(layout.analyzerBounds);
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
    layout.contentBounds = bounds;

    auto topAndGapBounds = bounds;
    layout.rackBounds = topAndGapBounds.removeFromBottom(metrics.rackHeight);
    topAndGapBounds.removeFromBottom(metrics.analyzerToRackGap);
    layout.horizontalDividerBounds = juce::Rectangle<int>(
        layout.contentBounds.getX(),
        layout.rackBounds.getY() - dividerThickness,
        layout.contentBounds.getWidth(),
        dividerThickness);
    layout.topSectionBounds = topAndGapBounds.withBottom(layout.horizontalDividerBounds.getY());

    const auto rawActionsBounds = layout.topSectionBounds.removeFromRight(metrics.sideStripWidth);
    const auto verticalGapBounds = layout.topSectionBounds.removeFromRight(metrics.analyzerToSideStripGap);
    const auto verticalDividerTop = layout.topSectionBounds.getY();
    const auto verticalDividerBottom = layout.horizontalDividerBounds.getY();
    const auto verticalDividerHeight = juce::jmax(0, verticalDividerBottom - verticalDividerTop);
    layout.verticalDividerBounds = juce::Rectangle<int>(dividerThickness, verticalDividerHeight)
                                       .withCentre({verticalGapBounds.getCentreX(), verticalDividerTop + verticalDividerHeight / 2});
    layout.analyzerBounds = layout.topSectionBounds.withRight(layout.verticalDividerBounds.getX());
    layout.actionsBounds = rawActionsBounds.withX(layout.verticalDividerBounds.getRight());
    return layout;
}
