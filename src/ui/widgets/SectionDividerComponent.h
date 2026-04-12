#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../theme/UiTheme.h"

class SectionDividerComponent final : public juce::Component {
public:
    enum class Orientation {
        horizontal,
        vertical
    };

    SectionDividerComponent(const Ui::Theme& themeToUse, Orientation orientationToUse);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void rebuildCachedLayer();

    const Ui::Theme& theme;
    Orientation orientation;
    juce::Image cachedLayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionDividerComponent)
};
