#pragma once

#include <array>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../AnalyzerSettingsActions.h"
#include "../../AnalyzerUiStateSource.h"
#include "../../UiTheme.h"
#include "SignalSlotActionButton.h"

class AnalyzerMeterControlsComponent final : public juce::Component,
                                             private AnalyzerUiStateSource::Listener {
public:
    AnalyzerMeterControlsComponent(AnalyzerUiStateSource &uiStateSource,
                                   AnalyzerSettingsActions &settingsActions,
                                   const Ui::Theme &theme);
    ~AnalyzerMeterControlsComponent() override;

    void resized() override;
    void paint(juce::Graphics &g) override;

private:
    void analyzerUiStateChanged(const Ui::AnalyzerUiState &state) override;
    void syncButtonStates(const Ui::AnalyzerUiState &state);
    void styleButton(juce::TextButton &button, bool isEnabled) const;
    void syncFreezeButtonState(const Ui::AnalyzerUiState &state);

    AnalyzerUiStateSource &uiStateSource;
    AnalyzerSettingsActions &settingsActions;
    const Ui::Theme &theme;
    juce::TextButton peakButton { "Peak" };
    juce::TextButton rmsButton { "RMS" };
    juce::TextButton holdButton { "Hold" };
    SignalSlotActionButton freezeButton;
    Ui::AnalyzerUiState currentState;
};
