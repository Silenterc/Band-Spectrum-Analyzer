#pragma once

#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiIcons.h"
#include "ui/theme/UiTheme.h"

class RasterRectanglePadButton final : public juce::Button {
public:
    RasterRectanglePadButton(const Ui::Theme& themeToUse, juce::String labelText);
    ~RasterRectanglePadButton() override = default;

    [[nodiscard]] juce::Rectangle<int> getPreferredBounds() const;
    [[nodiscard]] juce::Point<float> getVisualCenterOffset() const;
    [[nodiscard]] bool isActive() const;

    bool hitTest(int x, int y) override;
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    void resized() override;

    void setActive(bool shouldBeActive);
    void setLabel(juce::String newLabel);
    void setIcon(std::optional<Ui::IconId> newIcon);
    void setActiveMarkingColour(juce::Colour newActiveMarkingColour);

private:
    void rebuildCachedPadImages();

    const Ui::Theme& theme;
    juce::String label;
    std::optional<Ui::IconId> icon;
    bool active = false;
    std::optional<juce::Colour> activeMarkingColourOverride;
    juce::Rectangle<int> padBounds;
    juce::Rectangle<int> iconBounds;
    juce::Image cachedOffImage;
    juce::Image cachedOnImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RasterRectanglePadButton)
};
