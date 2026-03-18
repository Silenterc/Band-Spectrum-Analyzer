#include "SignalSelectionPopupContent.h"

namespace {
    class SignalSelectionRowButton final : public juce::Button {
    public:
        SignalSelectionRowButton(const Ui::Theme &themeToUse,
                                 const juce::String &labelToUse,
                                 const bool selectedToUse,
                                 const Analyzer::SignalSource sourceToUse,
                                 const Analyzer::SignalMode modeToUse)
            : juce::Button({}),
              source(sourceToUse),
              mode(modeToUse),
              theme(themeToUse),
              label(labelToUse),
              selected(selectedToUse) {
        }

        void paintButton(juce::Graphics &g, bool isMouseOverButton, bool) override {
            auto bounds = getLocalBounds().toFloat();
            const auto fill = selected ? theme.controlSurfaceHover.brighter(0.18f)
                                       : isMouseOverButton ? theme.controlSurfaceHover
                                                           : theme.controlSurface;
            g.setColour(isEnabled() ? fill : fill.withMultipliedAlpha(0.45f));
            g.fillRoundedRectangle(bounds, theme.metrics.slot.buttonCornerRadius);

            if (selected) {
                g.setColour(theme.controlBorder.brighter(0.6f));
                g.drawRoundedRectangle(bounds.reduced(0.5f), theme.metrics.slot.buttonCornerRadius, 1.5f);
            }

            g.setColour(isEnabled() ? theme.controlText : theme.subtleText.withMultipliedAlpha(0.75f));
            g.setFont(14.0f);
            g.drawText(label, bounds.reduced(10.0f, 0.0f).toNearestInt(), juce::Justification::centredLeft);
        }

        Analyzer::SignalSource source;
        Analyzer::SignalMode mode;

    private:
        const Ui::Theme &theme;
        juce::String label;
        bool selected = false;
    };
}

SignalSelectionPopupContent::SignalSelectionPopupContent(
    const Ui::Theme &themeToUse,
    const Analyzer::SignalSource currentSourceToUse,
    const Analyzer::SignalMode currentModeToUse,
    std::function<void(Analyzer::SignalSource, Analyzer::SignalMode)> onSelectToUse,
    std::function<void()> onDismissToUse)
    : theme(themeToUse),
      currentSource(currentSourceToUse),
      onSelect(std::move(onSelectToUse)),
      onDismiss(std::move(onDismissToUse)) {
    for (size_t index = 0; index < Ui::signalSlotOptions.size(); ++index) {
        const auto &option = Ui::signalSlotOptions[index];
        auto button = std::make_unique<SignalSelectionRowButton>(
            theme, option.label, currentSourceToUse == option.source && currentModeToUse == option.mode, option.source, option.mode);
        auto *buttonPtr = button.get();
        addAndMakeVisible(*button);
        button->onClick = [this, buttonPtr] {
            if (!buttonPtr->isEnabled())
                return;

            if (onSelect)
                onSelect(buttonPtr->source, buttonPtr->mode);

            if (auto *callout = findParentComponentOfClass<juce::CallOutBox>())
                callout->dismiss();
        };
        buttons[index] = std::move(button);
    }
}

SignalSelectionPopupContent::~SignalSelectionPopupContent() {
    if (onDismiss)
        onDismiss();
}

void SignalSelectionPopupContent::setAvailability(
    const std::function<bool(Analyzer::SignalSource, Analyzer::SignalMode)> &isAvailable) {
    for (size_t index = 0; index < buttons.size(); ++index) {
        const auto &option = Ui::signalSlotOptions[index];
        auto &button = buttons[index];
        const auto visible = option.source == currentSource;
        button->setVisible(visible);
        button->setEnabled(visible && isAvailable(option.source, option.mode));
    }
}

void SignalSelectionPopupContent::resized() {
    const auto &popupMetrics = theme.metrics.popup;
    auto bounds = getLocalBounds().reduced(static_cast<int>(popupMetrics.padding));

    for (size_t index = 0; index < buttons.size(); ++index) {
        const auto &option = Ui::signalSlotOptions[index];
        if (option.source != currentSource || !buttons[index]->isVisible())
            continue;

        buttons[index]->setBounds(bounds.removeFromTop(static_cast<int>(popupMetrics.rowHeight)));
        bounds.removeFromTop(static_cast<int>(popupMetrics.rowGap));
    }
}

int SignalSelectionPopupContent::getPreferredHeight() const {
    const auto &popupMetrics = theme.metrics.popup;
    return static_cast<int>(popupMetrics.padding * 2 + popupMetrics.rowHeight * 3 + popupMetrics.rowGap * 2);
}

int SignalSelectionPopupContent::getPreferredWidth() const {
    return 132;
}
