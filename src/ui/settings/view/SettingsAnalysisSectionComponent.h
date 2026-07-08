#pragma once

#include <array>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/settings/view/SettingsSectionFrameComponent.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterRectanglePadButton.h"

class SettingsAnalysisSectionComponent final : public juce::Component {
public:
    explicit SettingsAnalysisSectionComponent(const Ui::Theme& themeToUse);
    ~SettingsAnalysisSectionComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void createBandModeButtons();
    void selectBandModeButton(int selectedIndex);
    void layoutBandModeButtons(juce::Rectangle<int> bounds);

    const Ui::Theme& theme;
    SettingsSectionFrameComponent frame;
    std::array<std::unique_ptr<RasterRectanglePadButton>, 4> bandModeButtons;
    int selectedBandModeIndex = 2;
    juce::Rectangle<int> bandModeLabelBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsAnalysisSectionComponent)
};
