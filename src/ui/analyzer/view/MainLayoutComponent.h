#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../AnalyzerDataSource.h"
#include "../../AnalyzerSettingsActions.h"
#include "../../AnalyzerUiStateSource.h"
#include "../../SectionDividerComponent.h"
#include "../../UiTheme.h"
#include "AnalyzerSectionComponent.h"
#include "AnalyzerMeterControlsComponent.h"
#include "SignalRackComponent.h"

class MainLayoutComponent final : public juce::Component {
public:
    MainLayoutComponent(AnalyzerDataSource& dataSource,
                        AnalyzerUiStateSource& uiStateSource,
                        AnalyzerSettingsActions& settingsActions,
                        const Ui::Theme& theme);
    ~MainLayoutComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    struct Layout {
        juce::Rectangle<int> contentBounds;
        juce::Rectangle<int> topSectionBounds;
        juce::Rectangle<int> analyzerBounds;
        juce::Rectangle<int> actionsBounds;
        juce::Rectangle<int> rackBounds;
        juce::Rectangle<int> verticalDividerBounds;
        juce::Rectangle<int> horizontalDividerBounds;
    };

    Layout computeLayout() const;

    const Ui::Theme& theme;
    AnalyzerSectionComponent analyzerSectionComponent;
    SignalRackComponent signalRackComponent;
    AnalyzerMeterControlsComponent meterControlsComponent;
    SectionDividerComponent verticalSectionDivider;
    SectionDividerComponent horizontalSectionDivider;
};
