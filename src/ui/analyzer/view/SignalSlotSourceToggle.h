#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../UiTheme.h"
#include "../../../shared/SignalSlotConfiguration.h"

class SignalSlotSourceToggle final : public juce::Component {
public:
    explicit SignalSlotSourceToggle(const Ui::Theme &themeToUse);

    void setState(Analyzer::SignalSource sourceToUse, bool sidechainAvailableToUse);

    std::function<void(Analyzer::SignalSource)> onSourceSelected;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    enum class HoverHalf {
        none,
        main,
        sidechain
    };

    juce::Rectangle<float> getMainBounds() const;
    juce::Rectangle<float> getSidechainBounds() const;
    juce::Rectangle<float> getSwitchBounds() const;
    juce::Rectangle<int> getTopLabelBounds() const;
    juce::Rectangle<int> getBottomLabelBounds() const;
    HoverHalf getHoverHalf(juce::Point<float> position) const;
    void rebuildCachedSwitch();

    const Ui::Theme &theme;
    Analyzer::SignalSource source = Analyzer::SignalSource::main;
    bool sidechainAvailable = false;
    HoverHalf hoveredHalf = HoverHalf::none;
    juce::Image cachedSwitchImage;
};
