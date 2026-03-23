#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "UiTheme.h"

class EditorBackgroundComponent final : public juce::Component {
public:
    explicit EditorBackgroundComponent(const Ui::Theme& themeToUse);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void rebuildCachedLayer();

    const Ui::Theme& theme;
    juce::Image cachedLayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditorBackgroundComponent)
};
