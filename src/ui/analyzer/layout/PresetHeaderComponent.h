#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"
#include "ui/widgets/RasterIconButton.h"

class PresetHeaderComponent final : public juce::Component {
public:
    explicit PresetHeaderComponent(const Ui::Theme& themeToUse);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    [[nodiscard]] juce::Rectangle<int> getScaledTextBoxBounds(juce::Rectangle<int> availableBounds) const;
    [[nodiscard]] juce::Rectangle<int> getLogoBounds(juce::Rectangle<int> availableBounds) const;

    const Ui::Theme& theme;
    RasterIconButton previousButton;
    RasterIconButton nextButton;
    RasterIconButton resetButton;
    RasterIconButton saveButton;
    juce::Rectangle<int> logoBounds;
    juce::Rectangle<int> textBoxBounds;
    juce::Image cachedTextBoxImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetHeaderComponent)
};
