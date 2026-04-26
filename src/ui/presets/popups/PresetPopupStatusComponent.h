#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"

class PresetPopupStatusComponent final : public juce::Component {
public:
    explicit PresetPopupStatusComponent(const Ui::Theme& themeToUse);

    void setMessage(juce::String newMessage);
    [[nodiscard]] bool hasMessage() const;
    [[nodiscard]] int getPreferredHeight() const;

    void paint(juce::Graphics& g) override;

private:
    const Ui::Theme& theme;
    juce::String message;
};
