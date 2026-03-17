#include "AnalyzerPanelComponent.h"

AnalyzerPanelComponent::AnalyzerPanelComponent(AnalyzerDataSource &dataSourceToUse,
                                               AnalyzerUiStateSource &uiStateSourceToUse,
                                               AnalyzerSettingsActions &settingsActionsToUse,
                                               const Ui::Theme &themeToUse)
    : uiStateSource(uiStateSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse),
      analyzerComponent(dataSourceToUse, themeToUse),
      signalRackComponent(uiStateSourceToUse, settingsActionsToUse, themeToUse),
      meterControlsComponent(uiStateSourceToUse, settingsActionsToUse, themeToUse),
      freezeButton(themeToUse) {
    setOpaque(true);
    addAndMakeVisible(analyzerComponent);
    addAndMakeVisible(signalRackComponent);
    addAndMakeVisible(meterControlsComponent);
    addAndMakeVisible(freezeButton);

    freezeButton.onClick = [this] {
        const auto nextFrozen = !currentState.frozen;
        setLocalFrozenState(nextFrozen);
        settingsActions.setFreezeEnabled(nextFrozen);
    };
    freezeButton.setTooltip("Freeze analyzer");

    uiStateSource.addAnalyzerUiStateListener(*this);
    analyzerUiStateChanged(uiStateSource.getAnalyzerUiState());
}

AnalyzerPanelComponent::~AnalyzerPanelComponent() {
    uiStateSource.removeAnalyzerUiStateListener(*this);
}

void AnalyzerPanelComponent::paint(juce::Graphics &g) {
    g.fillAll(theme.editorBackground);
}

void AnalyzerPanelComponent::resized() {
    const auto &metrics = theme.metrics.panel;

    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop(metrics.headerHeight);
    const auto buttonSide = metrics.headerButtonSize;
    const auto buttonY = headerBounds.getY() + (headerBounds.getHeight() - buttonSide) / 2;
    freezeButton.setBounds(headerBounds.getRight() - buttonSide,
                           buttonY,
                           buttonSide,
                           buttonSide);
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
    freezeButton.setFrozen(state.frozen);
}

void AnalyzerPanelComponent::setLocalFrozenState(const bool isFrozen) {
    currentState.frozen = isFrozen;
    syncFreezeButtonState(currentState);
}
