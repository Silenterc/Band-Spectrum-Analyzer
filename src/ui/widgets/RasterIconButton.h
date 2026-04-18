#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiIcons.h"
#include "ui/theme/UiTheme.h"

class RasterIconButton final : public juce::Component,
                               public juce::SettableTooltipClient {
public:
    explicit RasterIconButton(const Ui::Theme& themeToUse);

    [[nodiscard]] int getPreferredSideLength() const;

    void setActive(bool shouldBeActive);
    void setIcon(Ui::IconId newIcon);
    void setScaleMultiplier(float newScaleMultiplier);
    void setIconScaleMultiplier(float newScaleMultiplier);

    std::function<void()> onClick;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

private:
    void rebuildCachedImages();

    const Ui::Theme& theme;
    Ui::IconId icon = Ui::IconId::left;
    bool active = false;
    bool pressed = false;
    float scaleMultiplier = 1.0f;
    float iconScaleMultiplier = 1.0f;
    juce::Rectangle<int> imageBounds;
    juce::Rectangle<int> iconBounds;
    juce::Image cachedOffImage;
    juce::Image cachedOnImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RasterIconButton)
};
