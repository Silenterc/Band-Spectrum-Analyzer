#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterHorizontalSliderComponent.h"
#include "ui/widgets/RasterKnobComponent.h"
#include "ui/widgets/RasterRectanglePadButton.h"
#include "ui/settings/view/SettingsSectionFrameComponent.h"

class SettingsPageComponent final : public juce::Component {
public:
    explicit SettingsPageComponent(const Ui::Theme& themeToUse);
    ~SettingsPageComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void rebuildCachedBackground();

    const Ui::Theme& theme;
    juce::Image cachedBackground;
    SettingsSectionFrameComponent mockFrequencyRangeFrame;
    RasterKnobComponent mockHoldTimeKnob;
    RasterHorizontalSliderComponent mockVisibleMinFrequencySlider;
    RasterRectanglePadButton mockBandModeButton;
    float mockHoldTimeMs = 750.0f;
    float mockVisibleMinFrequencyHz = 30.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPageComponent)
};
