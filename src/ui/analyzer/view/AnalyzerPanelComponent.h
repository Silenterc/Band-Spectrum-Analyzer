#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../AnalyzerDataSource.h"
#include "../../AnalyzerSettingsActions.h"
#include "../../AnalyzerUiStateSource.h"
#include "../../FlatButtonLookAndFeel.h"
#include "../../UiTheme.h"
#include "AnalyzerComponent.h"
#include "AnalyzerMeterControlsComponent.h"
#include "SignalRackComponent.h"

class AnalyzerPanelComponent final : public juce::Component,
                                     private AnalyzerUiStateSource::Listener {
public:
    AnalyzerPanelComponent(AnalyzerDataSource &dataSource,
                           AnalyzerUiStateSource &uiStateSource,
                           AnalyzerSettingsActions &settingsActions,
                           const Ui::Theme &theme);
    ~AnalyzerPanelComponent() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    void analyzerUiStateChanged(const Ui::AnalyzerUiState &state) override;
    void syncFreezeButtonState(const Ui::AnalyzerUiState &state);

    AnalyzerUiStateSource &uiStateSource;
    AnalyzerSettingsActions &settingsActions;
    const Ui::Theme &theme;
    FlatButtonLookAndFeel flatButtonLookAndFeel;
    AnalyzerComponent analyzerComponent;
    SignalRackComponent signalRackComponent;
    AnalyzerMeterControlsComponent meterControlsComponent;
    juce::TextButton freezeButton { "Freeze" };
    Ui::AnalyzerUiState currentState;
};
