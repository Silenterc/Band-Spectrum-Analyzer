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
    const bool sidechainAvailableToUse,
    const Analyzer::SignalSource currentSourceToUse,
    const Analyzer::SignalMode currentModeToUse,
    std::function<void(Analyzer::SignalSource, Analyzer::SignalMode)> onSelectToUse,
    std::function<void()> onDismissToUse)
    : theme(themeToUse),
      sidechainAvailable(sidechainAvailableToUse),
      onSelect(std::move(onSelectToUse)),
      onDismiss(std::move(onDismissToUse)) {
    addAndMakeVisible(mainLabel);
    mainLabel.setText("Main", juce::dontSendNotification);
    mainLabel.setFont(juce::FontOptions(11.0f));
    mainLabel.setColour(juce::Label::textColourId, theme.subtleText);
    mainLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(sidechainLabel);
    sidechainLabel.setText("Sidechain", juce::dontSendNotification);
    sidechainLabel.setFont(juce::FontOptions(11.0f));
    sidechainLabel.setColour(juce::Label::textColourId, theme.subtleText);
    sidechainLabel.setJustificationType(juce::Justification::centredLeft);

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
        const auto visible = !option.requiresSidechain || sidechainAvailable;
        button->setVisible(visible);
        button->setEnabled(visible && isAvailable(option.source, option.mode));
    }

    mainLabel.setVisible(sidechainAvailable);
    sidechainLabel.setVisible(sidechainAvailable);
}

void SignalSelectionPopupContent::resized() {
    const auto &popupMetrics = theme.metrics.popup;
    auto bounds = getLocalBounds().reduced(static_cast<int>(popupMetrics.padding));

    auto layoutSection = [&](juce::Label &header, const Analyzer::SignalSource source) {
        if (header.isVisible()) {
            header.setBounds(bounds.removeFromTop(static_cast<int>(popupMetrics.headerHeight)));
            bounds.removeFromTop(static_cast<int>(popupMetrics.rowGap));
        }

        bool anyVisible = false;
        for (size_t index = 0; index < buttons.size(); ++index) {
            const auto &option = Ui::signalSlotOptions[index];
            if (option.source != source || !buttons[index]->isVisible())
                continue;

            buttons[index]->setBounds(bounds.removeFromTop(static_cast<int>(popupMetrics.rowHeight)));
            bounds.removeFromTop(static_cast<int>(popupMetrics.rowGap));
            anyVisible = true;
        }

        if (anyVisible)
            bounds.removeFromTop(static_cast<int>(popupMetrics.sectionGap - popupMetrics.rowGap));
    };

    layoutSection(mainLabel, Analyzer::SignalSource::main);
    if (sidechainAvailable)
        layoutSection(sidechainLabel, Analyzer::SignalSource::sidechain);
}

int SignalSelectionPopupContent::getPreferredHeight() const {
    const auto &popupMetrics = theme.metrics.popup;
    if (!sidechainAvailable)
        return static_cast<int>(popupMetrics.padding * 2 + popupMetrics.rowHeight * 3 + popupMetrics.rowGap * 2);

    return static_cast<int>(popupMetrics.padding * 2
                            + popupMetrics.headerHeight * 2
                            + popupMetrics.rowHeight * 6
                            + popupMetrics.rowGap * 6
                            + popupMetrics.sectionGap);
}

int SignalSelectionPopupContent::getPreferredWidth() const {
    return 132;
}
