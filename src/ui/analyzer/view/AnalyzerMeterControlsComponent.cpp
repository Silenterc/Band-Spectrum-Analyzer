#include "AnalyzerMeterControlsComponent.h"

AnalyzerMeterControlsComponent::AnalyzerMeterControlsComponent(AnalyzerUiStateSource &uiStateSourceToUse,
                                                               AnalyzerSettingsActions &settingsActionsToUse,
                                                               const Ui::Theme &themeToUse)
    : uiStateSource(uiStateSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse) {
    addAndMakeVisible(peakButton);
    addAndMakeVisible(rmsButton);
    addAndMakeVisible(holdButton);

    for (auto *button : { &peakButton, &rmsButton, &holdButton })
        button->setWantsKeyboardFocus(false);

    peakButton.onClick = [this] {
        settingsActions.setShowPeakEnabled(!currentState.meterSettings.showPeak);
    };
    rmsButton.onClick = [this] {
        settingsActions.setShowRmsEnabled(!currentState.meterSettings.showRms);
    };
    holdButton.onClick = [this] {
        settingsActions.setShowHoldEnabled(!currentState.meterSettings.showHold);
    };

    peakButton.setTooltip("Show Peak");
    rmsButton.setTooltip("Show RMS");
    holdButton.setTooltip("Show Peak Hold");

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
    const auto buttonHeight = (bounds.getHeight() - gap * 2) / 3;

    peakButton.setBounds(bounds.removeFromTop(buttonHeight));
    bounds.removeFromTop(gap);
    rmsButton.setBounds(bounds.removeFromTop(buttonHeight));
    bounds.removeFromTop(gap);
    holdButton.setBounds(bounds);
}

void AnalyzerMeterControlsComponent::analyzerUiStateChanged(const Ui::AnalyzerUiState &state) {
    currentState = state;
    syncButtonStates(currentState);
}

void AnalyzerMeterControlsComponent::syncButtonStates(const Ui::AnalyzerUiState &state) {
    styleButton(peakButton, state.meterSettings.showPeak);
    styleButton(rmsButton, state.meterSettings.showRms);
    styleButton(holdButton, state.meterSettings.showHold);
}

void AnalyzerMeterControlsComponent::styleButton(juce::TextButton &button, const bool isEnabled) const {
    const auto fillColour = isEnabled ? theme.controlSurfaceHover.brighter(0.18f)
                                      : theme.controlSurface;
    const auto textColour = isEnabled ? theme.controlText
                                      : theme.controlText.withAlpha(0.94f);
    button.setColour(juce::TextButton::buttonColourId, fillColour);
    button.setColour(juce::TextButton::buttonOnColourId, fillColour);
    button.setColour(juce::TextButton::textColourOffId, textColour);
    button.setColour(juce::TextButton::textColourOnId, textColour);
    button.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
}
