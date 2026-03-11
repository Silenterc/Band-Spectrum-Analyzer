#include "AnalyzerPanelComponent.h"

AnalyzerPanelComponent::AnalyzerPanelComponent(AnalyzerDataSource &dataSourceToUse,
                                               AnalyzerUiStateSource &uiStateSourceToUse,
                                               AnalyzerSettingsActions &settingsActionsToUse,
                                               const Ui::Theme &themeToUse)
    : uiStateSource(uiStateSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse),
      flatButtonLookAndFeel(themeToUse),
      analyzerComponent(dataSourceToUse, themeToUse),
      signalRackComponent(uiStateSourceToUse, settingsActionsToUse, themeToUse),
      meterControlsComponent(uiStateSourceToUse, settingsActionsToUse, themeToUse) {
    addAndMakeVisible(analyzerComponent);
    addAndMakeVisible(signalRackComponent);
    addAndMakeVisible(meterControlsComponent);
    addAndMakeVisible(freezeButton);

    freezeButton.setClickingTogglesState(false);
    freezeButton.setLookAndFeel(&flatButtonLookAndFeel);
    freezeButton.onClick = [this] {
        settingsActions.setFreezeEnabled(!currentState.frozen);
    };

    uiStateSource.addAnalyzerUiStateListener(*this);
    analyzerUiStateChanged(uiStateSource.getAnalyzerUiState());
}

AnalyzerPanelComponent::~AnalyzerPanelComponent() {
    freezeButton.setLookAndFeel(nullptr);
    uiStateSource.removeAnalyzerUiStateListener(*this);
}

void AnalyzerPanelComponent::paint(juce::Graphics &g) {
    g.fillAll(theme.editorBackground);
}

void AnalyzerPanelComponent::resized() {
    const auto &metrics = theme.metrics.panel;

    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop(metrics.headerHeight);
    freezeButton.setBounds(headerBounds.removeFromRight(metrics.headerButtonWidth)
                                       .reduced(0, metrics.headerButtonVerticalInset));
    bounds.removeFromTop(metrics.headerBottomGap);
    auto rackBounds = bounds.removeFromBottom(metrics.rackHeight);
    bounds.removeFromBottom(metrics.analyzerToRackGap);
    auto meterBounds = rackBounds.removeFromRight(metrics.meterControlsWidth);
    rackBounds.removeFromRight(metrics.rackToMeterGap);
    analyzerComponent.setBounds(bounds);
    signalRackComponent.setBounds(rackBounds);
    meterControlsComponent.setBounds(meterBounds);
}

void AnalyzerPanelComponent::analyzerUiStateChanged(const Ui::AnalyzerUiState &state) {
    currentState = state;
    syncFreezeButtonState(currentState);
}

void AnalyzerPanelComponent::syncFreezeButtonState(const Ui::AnalyzerUiState &state) {
    freezeButton.setButtonText(state.frozen ? "Frozen" : "Freeze");
    freezeButton.setColour(juce::TextButton::buttonColourId, state.frozen ? theme.accentButtonActive : theme.controlSurface);
    freezeButton.setColour(juce::TextButton::buttonOnColourId, theme.accentButtonActive);
    freezeButton.setColour(juce::TextButton::textColourOffId, theme.controlText);
}
