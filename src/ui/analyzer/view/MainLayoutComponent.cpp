#include "MainLayoutComponent.h"

MainLayoutComponent::MainLayoutComponent(AnalyzerDataSource& dataSourceToUse,
                                         AnalyzerUiStateSource& uiStateSourceToUse,
                                         AnalyzerSettingsActions& settingsActionsToUse,
                                         const Ui::Theme& themeToUse)
    : theme(themeToUse),
      analyzerSectionComponent(dataSourceToUse, themeToUse),
      signalRackComponent(uiStateSourceToUse, settingsActionsToUse, themeToUse),
      meterControlsComponent(uiStateSourceToUse, settingsActionsToUse, themeToUse),
      verticalSectionDivider(themeToUse, SectionDividerComponent::Orientation::vertical),
      horizontalSectionDivider(themeToUse, SectionDividerComponent::Orientation::horizontal) {
    addAndMakeVisible(analyzerSectionComponent);
    // TODO: Re-enable the digital signal rack once the rasterized bottom-panel layout is ready.
    addChildComponent(signalRackComponent);
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
    auto bounds = getLocalBounds().reduced(woodSideInset, 0);
    layout.contentBounds = bounds;
    layout.rackBounds = bounds.removeFromBottom(metrics.rackHeight);
    const auto horizontalGapBounds = bounds.removeFromBottom(metrics.analyzerToRackGap);
    layout.topSectionBounds = bounds;
    layout.horizontalDividerBounds = juce::Rectangle<int>(layout.contentBounds.getWidth(), dividerThickness)
                                         .withCentre({layout.contentBounds.getCentreX(), horizontalGapBounds.getCentreY()});
    layout.topSectionBounds = layout.topSectionBounds.withBottom(layout.horizontalDividerBounds.getY());
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
