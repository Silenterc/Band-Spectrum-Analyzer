#include "AnalyzerMeterControlsComponent.h"

AnalyzerMeterControlsComponent::AnalyzerMeterControlsComponent(AnalyzerUiStateSource &uiStateSourceToUse,
                                                               AnalyzerSettingsActions &settingsActionsToUse,
                                                               const Ui::Theme &themeToUse)
    : uiStateSource(uiStateSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse),
      peakButton(themeToUse, "Peak"),
      rmsButton(themeToUse, "RMS"),
      holdButton(themeToUse, "Hold"),
      freezeButton(themeToUse, {}) {
    addAndMakeVisible(peakButton);
    addAndMakeVisible(rmsButton);
    addAndMakeVisible(holdButton);
    addAndMakeVisible(freezeButton);

    for (auto *button : { &peakButton, &rmsButton, &holdButton, &freezeButton })
        button->setWantsKeyboardFocus(false);

    peakButton.onClick = [this] {
        const auto nextEnabled = !currentState.meterSettings.showPeak;
        currentState.meterSettings.showPeak = nextEnabled;
        peakButton.setActive(nextEnabled);
        settingsActions.setShowPeakEnabled(nextEnabled);
    };
    rmsButton.onClick = [this] {
        const auto nextEnabled = !currentState.meterSettings.showRms;
        currentState.meterSettings.showRms = nextEnabled;
        rmsButton.setActive(nextEnabled);
        settingsActions.setShowRmsEnabled(nextEnabled);
    };
    holdButton.onClick = [this] {
        const auto nextEnabled = !currentState.meterSettings.showHold;
        currentState.meterSettings.showHold = nextEnabled;
        holdButton.setActive(nextEnabled);
        settingsActions.setShowHoldEnabled(nextEnabled);
    };
    freezeButton.onClick = [this] {
        const auto nextFrozen = !currentState.frozen;
        currentState.frozen = nextFrozen;
        freezeButton.setActive(nextFrozen);
        settingsActions.setFreezeEnabled(nextFrozen);
    };

    peakButton.setTooltip("Show Peak");
    rmsButton.setTooltip("Show RMS");
    holdButton.setTooltip("Show Peak Hold");
    freezeButton.setTooltip("Freeze analyzer");
    freezeButton.setOverlayIcon(PadButton::OverlayIcon::snowflake);

    uiStateSource.addAnalyzerUiStateListener(*this);
    analyzerUiStateChanged(uiStateSource.getAnalyzerUiState());
}

AnalyzerMeterControlsComponent::~AnalyzerMeterControlsComponent() {
    uiStateSource.removeAnalyzerUiStateListener(*this);
}

void AnalyzerMeterControlsComponent::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
}

void AnalyzerMeterControlsComponent::resized() {
    const auto &metrics = theme.metrics.meterControls;
    auto bounds = getLocalBounds().reduced(0, metrics.verticalPadding);
    const auto gap = metrics.buttonGap;
    const auto peakHeight = peakButton.getPreferredHeight(bounds.getWidth());
    const auto rmsHeight = rmsButton.getPreferredHeight(bounds.getWidth());
    const auto holdHeight = holdButton.getPreferredHeight(bounds.getWidth());
    const auto freezeHeight = freezeButton.getPreferredHeight(bounds.getWidth());
    const auto totalButtonsHeight = peakHeight + rmsHeight + holdHeight + freezeHeight + gap * 3;
    const auto topInset = juce::jmax(0, (bounds.getHeight() - totalButtonsHeight) / 2);
    bounds.removeFromTop(topInset);

    peakButton.setBounds(bounds.removeFromTop(peakHeight));
    bounds.removeFromTop(gap);
    rmsButton.setBounds(bounds.removeFromTop(rmsHeight));
    bounds.removeFromTop(gap);
    holdButton.setBounds(bounds.removeFromTop(holdHeight));
    bounds.removeFromTop(gap);
    freezeButton.setBounds(bounds.removeFromTop(freezeHeight));
}

void AnalyzerMeterControlsComponent::analyzerUiStateChanged(const Ui::AnalyzerUiState &state) {
    currentState = state;
    syncButtonStates(currentState);
}

void AnalyzerMeterControlsComponent::syncButtonStates(const Ui::AnalyzerUiState &state) {
    peakButton.setActive(state.meterSettings.showPeak);
    rmsButton.setActive(state.meterSettings.showRms);
    holdButton.setActive(state.meterSettings.showHold);
    freezeButton.setActive(state.frozen);
}
