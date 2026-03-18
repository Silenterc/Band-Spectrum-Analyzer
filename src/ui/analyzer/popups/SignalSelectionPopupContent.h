#pragma once

#include <array>
#include <functional>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../UiTheme.h"
#include "../model/SignalSlotOptions.h"

class SignalSelectionPopupContent final : public juce::Component {
public:
    SignalSelectionPopupContent(const Ui::Theme &themeToUse,
                                Analyzer::SignalSource currentSourceToUse,
                                Analyzer::SignalMode currentModeToUse,
                                std::function<void(Analyzer::SignalSource, Analyzer::SignalMode)> onSelectToUse,
                                std::function<void()> onDismissToUse);
    ~SignalSelectionPopupContent() override;

    void setAvailability(const std::function<bool(Analyzer::SignalSource, Analyzer::SignalMode)> &isAvailable);

    void resized() override;

    int getPreferredHeight() const;
    int getPreferredWidth() const;

private:
    const Ui::Theme &theme;
    Analyzer::SignalSource currentSource = Analyzer::SignalSource::main;
    std::array<std::unique_ptr<juce::Button>, Ui::signalSlotOptions.size()> buttons;
    std::function<void(Analyzer::SignalSource, Analyzer::SignalMode)> onSelect;
    std::function<void()> onDismiss;
};
