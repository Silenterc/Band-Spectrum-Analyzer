#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../AnalyzerDataSource.h"
#include "../../AnalyzerSettingsActions.h"
#include "../../AnalyzerUiStateSource.h"
#include "../../UiTheme.h"
#include "AnalyzerComponent.h"
#include "AnalyzerMeterControlsComponent.h"
#include "SignalRackComponent.h"

class AnalyzerPanelComponent final : public juce::Component {
public:
    AnalyzerPanelComponent(AnalyzerDataSource &dataSource,
                           AnalyzerUiStateSource &uiStateSource,
                           AnalyzerSettingsActions &settingsActions,
                           const Ui::Theme &theme);
    ~AnalyzerPanelComponent() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    const Ui::Theme &theme;
    AnalyzerComponent analyzerComponent;
    SignalRackComponent signalRackComponent;
    AnalyzerMeterControlsComponent meterControlsComponent;
};
