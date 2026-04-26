#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "display/analyzer/contracts/AnalyzerRawTraceSource.h"
#include "ui/analyzer/contracts/AnalyzerSettingsActions.h"
#include "ui/analyzer/contracts/AnalyzerUiSnapshotSource.h"
#include "ui/presets/contracts/PresetActions.h"
#include "ui/presets/contracts/PresetUiSnapshotSource.h"
#include "ui/widgets/SectionDividerComponent.h"
#include "ui/theme/UiTheme.h"
#include "ui/presets/view/PresetHeaderComponent.h"
#include "ui/analyzer/plot/view/AnalyzerSectionComponent.h"
#include "ui/analyzer/controls/view/AnalyzerMeterControlsComponent.h"
#include "ui/analyzer/rack/view/SignalRackComponent.h"

class MainLayoutComponent final : public juce::Component {
public:
    MainLayoutComponent(AnalyzerRawTraceSource& rawTraceSource,
                        AnalyzerUiSnapshotSource& snapshotSource,
                        PresetUiSnapshotSource& presetUiSnapshotSource,
                        AnalyzerSettingsActions& settingsActions,
                        PresetActions& presetActions,
                        const Ui::Theme& theme);
    ~MainLayoutComponent() override = default;

    void resized() override;

private:
    struct Layout {
        juce::Rectangle<int> analyzerBounds;
        juce::Rectangle<int> presetHeaderBounds;
        juce::Rectangle<int> actionsBounds;
        juce::Rectangle<int> rackBounds;
        juce::Rectangle<int> verticalDividerBounds;
        juce::Rectangle<int> horizontalDividerBounds;
    };

    Layout computeLayout() const;

    const Ui::Theme& theme;
    PresetHeaderComponent presetHeaderComponent;
    AnalyzerSectionComponent analyzerSectionComponent;
    SignalRackComponent signalRackComponent;
    AnalyzerMeterControlsComponent meterControlsComponent;
    SectionDividerComponent verticalSectionDivider;
    SectionDividerComponent horizontalSectionDivider;
};
