#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiIcons.h"
#include "ui/theme/UiTheme.h"

class RasterIconButton final : public juce::Button {
public:
    explicit RasterIconButton(const Ui::Theme& themeToUse);

    [[nodiscard]] int getPreferredSideLength() const;

    void setActive(bool shouldBeActive);
    void setIcon(Ui::IconId newIcon);
    void setScaleMultiplier(float newScaleMultiplier);
    void setIconScaleMultiplier(float newScaleMultiplier);

    std::function<void()> onPressed;

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    void resized() override;
    void buttonStateChanged() override;

private:
    void rebuildCachedImages();

    const Ui::Theme& theme;
    Ui::IconId icon = Ui::IconId::left;
    bool active = false;
    float scaleMultiplier = 1.0f;
    float iconScaleMultiplier = 1.0f;
    juce::Rectangle<int> imageBounds;
    juce::Rectangle<int> iconBounds;
    juce::Image cachedOffImage;
    juce::Image cachedOnImage;
    bool wasDown = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RasterIconButton)
};
