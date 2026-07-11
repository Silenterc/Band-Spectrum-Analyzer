#pragma once

#include <array>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "shared/ParameterOptionCatalog.h"
#include "ui/editor/contracts/EditorPresentationActions.h"
#include "ui/editor/state/EditorPresentationState.h"
#include "ui/settings/view/SettingsSectionFrameComponent.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterRectanglePadButton.h"

class SettingsUiSectionComponent final : public juce::Component {
public:
    SettingsUiSectionComponent(EditorPresentationActions& presentationActionsToUse, const Ui::Theme& themeToUse);
    ~SettingsUiSectionComponent() override = default;

    void applyPresentationState(const Ui::EditorPresentationState& state);

    void resized() override;

private:
    void createScaleButtons();
    void selectScaleButton(int selectedIndex);
    void layoutScaleButtons(juce::Rectangle<int> bounds);

    EditorPresentationActions& presentationActions;
    const Ui::Theme& theme;
    SettingsSectionFrameComponent frame;
    std::array<std::unique_ptr<RasterRectanglePadButton>, Shared::uiScaleChoices.size()> scaleButtons;
    int selectedScaleIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsUiSectionComponent)
};
