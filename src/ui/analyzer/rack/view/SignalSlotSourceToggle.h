#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"
#include "shared/SignalSlotConfiguration.h"

class SignalSlotSourceToggle final : public juce::Component {
public:
    explicit SignalSlotSourceToggle(const Ui::Theme &themeToUse);

    void setState(Analyzer::SignalSource sourceToUse, bool sidechainAvailableToUse);

    std::function<void(Analyzer::SignalSource)> onSourceSelected;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent &) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    juce::Rectangle<float> getSwitchBounds() const;
    juce::Rectangle<int> getTopLabelBounds() const;
    juce::Rectangle<int> getBottomLabelBounds() const;
    void rebuildCachedSwitch();

    const Ui::Theme &theme;
    Analyzer::SignalSource source = Analyzer::SignalSource::main;
    bool sidechainAvailable = false;
    juce::Image cachedSwitchImage;
};
