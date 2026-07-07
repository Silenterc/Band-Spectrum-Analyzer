#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"

class SettingsSectionFrameComponent final : public juce::Component {
public:
    SettingsSectionFrameComponent(const Ui::Theme& themeToUse, juce::String titleText);
    ~SettingsSectionFrameComponent() override = default;

    void paint(juce::Graphics& g) override;
    bool hitTest(int x, int y) override;

    void setTitle(juce::String newTitle);

private:
    const Ui::Theme& theme;
    juce::String title;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsSectionFrameComponent)
};
