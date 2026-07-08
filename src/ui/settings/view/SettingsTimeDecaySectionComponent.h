#pragma once

#include <array>
#include <cstddef>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/settings/view/SettingsSectionFrameComponent.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterKnobComponent.h"

class SettingsTimeDecaySectionComponent final : public juce::Component {
public:
    explicit SettingsTimeDecaySectionComponent(const Ui::Theme& themeToUse);
    ~SettingsTimeDecaySectionComponent() override = default;

    void resized() override;

private:
    void createKnobs();
    void configureKnob(std::size_t index);
    void layoutKnobs(juce::Rectangle<int> bounds);

    const Ui::Theme& theme;
    SettingsSectionFrameComponent frame;
    std::array<std::unique_ptr<RasterKnobComponent>, 4> knobs;
    std::array<float, 4> values{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsTimeDecaySectionComponent)
};
