#pragma once

#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../theme/UiTheme.h"

class PadButton final : public juce::Button {
public:
    enum class AssetStyle {
        standard,
        freeze
    };

    enum class OverlayIcon {
        none,
        headphones,
        power,
        settings,
        snowflake
    };

    PadButton(const Ui::Theme& themeToUse, juce::String labelText);

    int getPreferredHeight(int availableWidth) const;
    bool hitTest(int x, int y) override;
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    void resized() override;

    void setActive(bool shouldBeActive);
    void setActiveMarkingColour(juce::Colour newActiveMarkingColour);
    void setAssetStyle(AssetStyle newAssetStyle);
    void setDrawsPad(bool shouldDrawPad);
    void setOverlayIconScaleMultiplier(float newOverlayIconScaleMultiplier);
    void setScaleMultiplier(float newScaleMultiplier);
    void setLabel(juce::String newLabel);
    void setOverlayIcon(OverlayIcon newOverlayIcon);

private:
    const juce::Image& getResolvedOnImage() const;
    juce::Rectangle<int> getTargetPadBounds(juce::Rectangle<int> availableBounds) const;
    void rebuildCachedPadImages();

    const Ui::Theme& theme;
    juce::String label;
    bool active = false;
    AssetStyle assetStyle = AssetStyle::standard;
    float scaleMultiplier = 1.0f;
    std::optional<juce::Colour> activeMarkingColourOverride;
    bool drawsPad = true;
    OverlayIcon overlayIcon = OverlayIcon::none;
    float overlayIconScaleMultiplier = 1.0f;
    juce::Rectangle<int> padBounds;
    juce::Rectangle<int> overlayIconBounds;
    juce::Image cachedOffImage;
    juce::Image cachedOnImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PadButton)
};
