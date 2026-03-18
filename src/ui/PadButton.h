#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "UiTheme.h"

class PadButton final : public juce::Component,
                        public juce::SettableTooltipClient {
public:
    enum class AssetStyle {
        standard,
        freeze
    };

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
    void setAssetStyle(AssetStyle newAssetStyle);
    void setLabel(juce::String newLabel);
    void setOverlayIcon(OverlayIcon newOverlayIcon);

    std::function<void()> onClick;

private:
    static const juce::Image& getOffImage();
    static const juce::Image& getOnImage();
    static const juce::Image& getFreezeOnImage();
    const juce::Image& getResolvedOnImage() const;
    void rebuildCachedPadImages();

    void mouseUp(const juce::MouseEvent& event) override;

    const Ui::Theme& theme;
    juce::String label;
    bool active = false;
    AssetStyle assetStyle = AssetStyle::standard;
    OverlayIcon overlayIcon = OverlayIcon::none;
    juce::Rectangle<int> padBounds;
    juce::Image cachedOffImage;
    juce::Image cachedOnImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadButton)
};
