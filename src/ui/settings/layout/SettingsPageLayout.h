#pragma once

#include <juce_graphics/juce_graphics.h>

#include "ui/theme/UiTheme.h"

namespace Ui {
    struct SettingsPageLayout {
        juce::Rectangle<int> contentBounds;
        juce::Rectangle<int> analysisSectionBounds;
        juce::Rectangle<int> timeDecaySectionBounds;
        juce::Rectangle<int> gridSectionBounds;
    };

    class SettingsPageLayoutBuilder {
    public:
        static SettingsPageLayout build(juce::Rectangle<int> bounds, const Theme& theme);
    };
}
