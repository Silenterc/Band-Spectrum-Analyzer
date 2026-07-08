#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/settings/view/SettingsSectionFrameComponent.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterHorizontalSliderComponent.h"
#include "ui/widgets/RasterRectanglePadButton.h"

class SettingsFrequencyRangeSectionComponent final : public juce::Component {
public:
    explicit SettingsFrequencyRangeSectionComponent(const Ui::Theme& themeToUse);
    ~SettingsFrequencyRangeSectionComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void configureSliders();
    void layoutControls(juce::Rectangle<int> bounds);
    void updateCustomRangeButton();

    const Ui::Theme& theme;
    SettingsSectionFrameComponent frame;
    RasterRectanglePadButton customRangeButton;
    RasterHorizontalSliderComponent visibleMinFrequencySlider;
    RasterHorizontalSliderComponent visibleMaxFrequencySlider;
    bool customRangeEnabled = true;
    float visibleMinFrequencyHz = 30.0f;
    float visibleMaxFrequencyHz = 18000.0f;
    juce::Rectangle<int> customRangeLabelBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsFrequencyRangeSectionComponent)
};
