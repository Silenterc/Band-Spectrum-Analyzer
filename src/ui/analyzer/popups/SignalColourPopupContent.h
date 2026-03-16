#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../UiTheme.h"

class SignalColourPopupContent final : public juce::Component {
public:
    SignalColourPopupContent(const Ui::Theme &themeToUse,
                             std::function<void(int)> onSelectToUse,
                             std::function<void()> onDismissToUse);
    ~SignalColourPopupContent() override;

    void addColourButton(juce::Colour colour, bool selected, bool enabled, int colourIndex);

    void resized() override;

    int getPreferredWidth() const;
    int getPreferredHeight() const;

    std::function<void(int)> onSelect;

private:
    const Ui::Theme &theme;
    std::vector<std::unique_ptr<juce::Button>> colourButtons;
    std::function<void()> onDismiss;
};
