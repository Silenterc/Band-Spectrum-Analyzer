#pragma once

#include <array>
#include <functional>
#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/theme/UiTheme.h"
#include "ui/analyzer/rack/model/SignalSlotOptions.h"

class SignalModeSelectionPopupContent final : public juce::Component {
public:
    SignalModeSelectionPopupContent(const Ui::Theme &themeToUse,
                                    Analyzer::SignalSource currentSourceToUse,
                                    Analyzer::SignalMode currentModeToUse,
                                    std::function<void(Analyzer::SignalMode)> onSelectToUse,
                                    std::function<void()> onDismissToUse);
    ~SignalModeSelectionPopupContent() override;

    void setAvailability(const std::function<bool(Analyzer::SignalMode)> &isAvailable);

    void resized() override;

    int getPreferredHeight() const;
    int getPreferredWidth() const;

private:
    void paint(juce::Graphics &g) override;

    const Ui::Theme &theme;
    Analyzer::SignalSource currentSource = Analyzer::SignalSource::main;
    std::array<std::unique_ptr<juce::Button>, Ui::signalSlotOptions.size()> buttons;
    std::function<void(Analyzer::SignalMode)> onSelect;
    std::function<void()> onDismiss;
};
