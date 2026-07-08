#pragma once

#include <array>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/settings/view/SettingsSectionFrameComponent.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterRectanglePadButton.h"

class SettingsUiSectionComponent final : public juce::Component {
public:
    explicit SettingsUiSectionComponent(const Ui::Theme& themeToUse);
    ~SettingsUiSectionComponent() override = default;

    void resized() override;

private:
    void createScaleButtons();
    void selectScaleButton(int selectedIndex);
    void layoutScaleButtons(juce::Rectangle<int> bounds);

    const Ui::Theme& theme;
    SettingsSectionFrameComponent frame;
    std::array<std::unique_ptr<RasterRectanglePadButton>, 3> scaleButtons;
    int selectedScaleIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsUiSectionComponent)
};
