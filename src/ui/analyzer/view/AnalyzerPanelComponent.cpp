#include "AnalyzerPanelComponent.h"

AnalyzerPanelComponent::AnalyzerPanelComponent(AnalyzerDataSource &dataSourceToUse,
                                               AnalyzerUiStateSource &uiStateSourceToUse,
                                               AnalyzerSettingsActions &settingsActionsToUse,
                                               const Ui::Theme &themeToUse)
    : theme(themeToUse),
      analyzerComponent(dataSourceToUse, themeToUse),
      signalRackComponent(uiStateSourceToUse, settingsActionsToUse, themeToUse),
      meterControlsComponent(uiStateSourceToUse, settingsActionsToUse, themeToUse) {
    // TODO: Re-enable the digital analyzer once the rasterized screen layout is ready.
    addChildComponent(analyzerComponent);
    // TODO: Re-enable the digital signal rack once the rasterized bottom-panel layout is ready.
    addChildComponent(signalRackComponent);
    addAndMakeVisible(meterControlsComponent);
}

AnalyzerPanelComponent::~AnalyzerPanelComponent() {
}

void AnalyzerPanelComponent::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
}

void AnalyzerPanelComponent::resized() {
    const auto &metrics = theme.metrics.panel;

    auto bounds = getLocalBounds();
    auto rackBounds = bounds.removeFromBottom(metrics.rackHeight);
    bounds.removeFromBottom(metrics.analyzerToRackGap);
    auto sideStripBounds = bounds.removeFromRight(metrics.sideStripWidth);
    bounds.removeFromRight(metrics.analyzerToSideStripGap);
    analyzerComponent.setBounds(bounds);
    signalRackComponent.setBounds(rackBounds);
    meterControlsComponent.setBounds(sideStripBounds);
}
