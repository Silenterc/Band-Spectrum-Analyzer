#include "AnalyzerMeterControlsComponent.h"

#include "../../UiButtonDrawing.h"

AnalyzerMeterControlsComponent::AnalyzerMeterControlsComponent(AnalyzerUiStateSource &uiStateSourceToUse,
                                                               AnalyzerSettingsActions &settingsActionsToUse,
                                                               const Ui::Theme &themeToUse)
    : uiStateSource(uiStateSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse),
      freezeButton(themeToUse) {
    addAndMakeVisible(peakButton);
    addAndMakeVisible(rmsButton);
    addAndMakeVisible(holdButton);
    addAndMakeVisible(freezeButton);

    for (auto *button : { &peakButton, &rmsButton, &holdButton })
        button->setWantsKeyboardFocus(false);

    freezeButton.setWantsKeyboardFocus(false);

    peakButton.onClick = [this] {
        settingsActions.setShowPeakEnabled(!currentState.meterSettings.showPeak);
    };
    rmsButton.onClick = [this] {
        settingsActions.setShowRmsEnabled(!currentState.meterSettings.showRms);
    };
    holdButton.onClick = [this] {
        settingsActions.setShowHoldEnabled(!currentState.meterSettings.showHold);
    };
    freezeButton.onClick = [this] {
        const auto nextFrozen = !currentState.frozen;
        currentState.frozen = nextFrozen;
        syncFreezeButtonState(currentState);
        settingsActions.setFreezeEnabled(nextFrozen);
    };

    peakButton.setTooltip("Show Peak");
    rmsButton.setTooltip("Show RMS");
    holdButton.setTooltip("Show Peak Hold");
    freezeButton.setTooltip("Freeze analyzer");

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
    const auto buttonHeight = (bounds.getHeight() - gap * 3) / 4;

    peakButton.setBounds(bounds.removeFromTop(buttonHeight));
    bounds.removeFromTop(gap);
    rmsButton.setBounds(bounds.removeFromTop(buttonHeight));
    bounds.removeFromTop(gap);
    holdButton.setBounds(bounds.removeFromTop(buttonHeight));
    bounds.removeFromTop(gap);
    freezeButton.setBounds(bounds);
}

void AnalyzerMeterControlsComponent::analyzerUiStateChanged(const Ui::AnalyzerUiState &state) {
    currentState = state;
    syncButtonStates(currentState);
}

void AnalyzerMeterControlsComponent::syncButtonStates(const Ui::AnalyzerUiState &state) {
    styleButton(peakButton, state.meterSettings.showPeak);
    styleButton(rmsButton, state.meterSettings.showRms);
    styleButton(holdButton, state.meterSettings.showHold);
    syncFreezeButtonState(state);
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

void AnalyzerMeterControlsComponent::syncFreezeButtonState(const Ui::AnalyzerUiState &state) {
    const auto freezeIconStyle = Ui::getIconActionButtonStyle(theme, state.frozen, false);
    SignalSlotActionButton::Style freezeStyle;
    freezeStyle.content = SignalSlotActionButton::Content::snowflake;
    freezeStyle.fill = freezeIconStyle.fill;
    freezeStyle.hoverFill = Ui::getIconActionButtonStyle(theme, state.frozen, true).fill;
    freezeStyle.foreground = freezeIconStyle.icon;
    freezeButton.setStyle(freezeStyle);
}
