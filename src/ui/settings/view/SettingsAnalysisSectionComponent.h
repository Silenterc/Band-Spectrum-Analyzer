#pragma once

#include <array>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "shared/ParameterOptionCatalog.h"
#include "ui/analyzer/contracts/AnalyzerSettingsActions.h"
#include "ui/analyzer/state/AnalyzerUiSnapshot.h"
#include "ui/settings/view/SettingsSectionFrameComponent.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterRectanglePadButton.h"

class SettingsAnalysisSectionComponent final : public juce::Component {
public:
    SettingsAnalysisSectionComponent(AnalyzerSettingsActions& settingsActionsToUse, const Ui::Theme& themeToUse);
    ~SettingsAnalysisSectionComponent() override = default;

    void applySnapshot(const Ui::AnalyzerUiSnapshot& snapshot);

    void resized() override;

private:
    void createBandModeButtons();
    void selectBandModeButton(int selectedIndex);
    void layoutBandModeButtons(juce::Rectangle<int> bounds);

    AnalyzerSettingsActions& settingsActions;
    const Ui::Theme& theme;
    SettingsSectionFrameComponent frame;
    std::array<std::unique_ptr<RasterRectanglePadButton>, Shared::bandModeChoices.size()> bandModeButtons;
    int selectedBandModeIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsAnalysisSectionComponent)
};
