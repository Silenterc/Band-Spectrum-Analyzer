#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "UiTheme.h"

class PadButton final : public juce::Component,
                        public juce::SettableTooltipClient {
public:
    enum class OverlayIcon {
        none,
        snowflake
    };

    PadButton(const Ui::Theme& themeToUse, juce::String labelText);

    int getPreferredHeight(int availableWidth) const;
    bool hitTest(int x, int y) override;
    void paint(juce::Graphics& g) override;
    void resized() override;

    void setActive(bool shouldBeActive);
    void setLabel(juce::String newLabel);
    void setOverlayIcon(OverlayIcon newOverlayIcon);

    std::function<void()> onClick;

private:
    static const juce::Image& getOffImage();
    static const juce::Image& getOnImage();
    void rebuildCachedPadImages();

    void mouseUp(const juce::MouseEvent& event) override;

    const Ui::Theme& theme;
    juce::String label;
    bool active = false;
    OverlayIcon overlayIcon = OverlayIcon::none;
    juce::Rectangle<int> padBounds;
    juce::Image cachedOffImage;
    juce::Image cachedOnImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadButton)
};
