#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"

class BrandLogoComponent final : public juce::Component {
public:
    explicit BrandLogoComponent(const Ui::Theme& themeToUse);
    ~BrandLogoComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    static void drawLogo(juce::Graphics& g, const juce::Rectangle<int>& bounds);
    [[nodiscard]] static juce::Rectangle<int> getLogoBounds(juce::Rectangle<int> availableBounds,
                                                            const Ui::Theme& theme);

private:
    const Ui::Theme& theme;
    juce::Rectangle<int> logoBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrandLogoComponent)
};
