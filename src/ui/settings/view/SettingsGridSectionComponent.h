#pragma once

#include <array>
#include <cstddef>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/analyzer/contracts/AnalyzerSettingsActions.h"
#include "ui/analyzer/state/AnalyzerUiSnapshot.h"
#include "ui/settings/view/SettingsSectionFrameComponent.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterKnobComponent.h"

class SettingsGridSectionComponent final : public juce::Component {
public:
    SettingsGridSectionComponent(AnalyzerSettingsActions& settingsActionsToUse, const Ui::Theme& themeToUse);
    ~SettingsGridSectionComponent() override = default;

    void applySnapshot(const Ui::AnalyzerUiSnapshot& snapshot);

    void resized() override;

private:
    void createKnobs();
    void configureKnob(std::size_t index);
    void layoutKnobs(juce::Rectangle<int> bounds);

    AnalyzerSettingsActions& settingsActions;
    const Ui::Theme& theme;
    SettingsSectionFrameComponent frame;
    std::array<std::unique_ptr<RasterKnobComponent>, 3> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsGridSectionComponent)
};
