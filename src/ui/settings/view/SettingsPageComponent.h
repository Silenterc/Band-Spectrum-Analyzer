#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"
#include "ui/settings/view/SettingsAnalysisSectionComponent.h"
#include "ui/settings/view/SettingsTimeDecaySectionComponent.h"

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
    SettingsAnalysisSectionComponent analysisSection;
    SettingsTimeDecaySectionComponent timeDecaySection;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPageComponent)
};
