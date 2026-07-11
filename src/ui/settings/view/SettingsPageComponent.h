#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/analyzer/contracts/AnalyzerSettingsActions.h"
#include "ui/analyzer/contracts/AnalyzerUiSnapshotSource.h"
#include "ui/editor/contracts/EditorPresentationActions.h"
#include "ui/editor/contracts/EditorPresentationStateSource.h"
#include "ui/theme/UiTheme.h"
#include "ui/settings/view/SettingsAnalysisSectionComponent.h"
#include "ui/settings/view/SettingsFrequencyRangeSectionComponent.h"
#include "ui/settings/view/SettingsGridSectionComponent.h"
#include "ui/settings/view/SettingsTimeDecaySectionComponent.h"
#include "ui/settings/view/SettingsUiSectionComponent.h"

class SettingsPageComponent final : public juce::Component,
                                    private AnalyzerUiSnapshotSource::Listener,
                                    private EditorPresentationStateSource::Listener {
public:
    SettingsPageComponent(AnalyzerUiSnapshotSource& uiSnapshotSourceToUse,
                          AnalyzerSettingsActions& settingsActionsToUse,
                          EditorPresentationStateSource& presentationStateSourceToUse,
                          EditorPresentationActions& presentationActionsToUse,
                          const Ui::Theme& themeToUse);
    ~SettingsPageComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot& snapshot) override;
    void editorPresentationStateChanged(const Ui::EditorPresentationState& state) override;
    void rebuildCachedBackground();

    AnalyzerUiSnapshotSource& uiSnapshotSource;
    EditorPresentationStateSource& presentationStateSource;
    const Ui::Theme& theme;
    juce::Image cachedBackground;
    SettingsAnalysisSectionComponent analysisSection;
    SettingsTimeDecaySectionComponent timeDecaySection;
    SettingsGridSectionComponent gridSection;
    SettingsUiSectionComponent uiSection;
    SettingsFrequencyRangeSectionComponent frequencyRangeSection;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPageComponent)
};
