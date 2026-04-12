#include "SignalModeSelectionPopupContent.h"

#include "ui/theme/PopupChrome.h"
#include "ui/theme/UiRasterAssets.h"

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
            const auto &popupMetrics = theme.metrics.popup;
            auto bounds = getLocalBounds().toFloat();

            if (selected) {
                Ui::drawAssetWithin(g,
                                    Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::screen),
                                    getLocalBounds());
            } else {
                auto fill = theme.controlSurface;
                if (isMouseOverButton)
                    fill = theme.controlSurfaceHover;

                g.setColour(isEnabled() ? fill : fill.withMultipliedAlpha(popupMetrics.rowDisabledAlpha));
                g.fillRoundedRectangle(bounds, popupMetrics.rowCornerRadius);
                g.setColour(theme.sectionDividerHighlight.withMultipliedAlpha(popupMetrics.rowOutlineAlpha));
                g.drawRoundedRectangle(bounds.reduced(0.5f), popupMetrics.rowCornerRadius, 1.0f);
            }

            g.setColour(selected ? theme.hardwareMarkingDark
                                 : (isEnabled() ? theme.axisText.brighter(0.18f)
                                                : theme.axisText.withMultipliedAlpha(0.55f)));
            g.setFont(juce::FontOptions(popupMetrics.rowTextFontHeight, juce::Font::bold));
            g.drawText(label, bounds.reduced(10.0f, 0.0f).toNearestInt(), juce::Justification::centred);
        }

        Analyzer::SignalSource source;
        Analyzer::SignalMode mode;

    private:
        const Ui::Theme &theme;
        juce::String label;
        bool selected = false;
    };
}

SignalModeSelectionPopupContent::SignalModeSelectionPopupContent(
    const Ui::Theme &themeToUse,
    const Analyzer::SignalSource currentSourceToUse,
    const Analyzer::SignalMode currentModeToUse,
    std::function<void(Analyzer::SignalMode)> onSelectToUse,
    std::function<void()> onDismissToUse)
    : theme(themeToUse),
      currentSource(currentSourceToUse),
      onSelect(std::move(onSelectToUse)),
      onDismiss(std::move(onDismissToUse)) {
    for (size_t index = 0; index < Ui::signalSlotOptions.size(); ++index) {
        const auto &option = Ui::signalSlotOptions[index];
        auto button = std::make_unique<SignalSelectionRowButton>(
            theme,
            option.modeLabel,
            currentSourceToUse == option.source && currentModeToUse == option.mode,
            option.source,
            option.mode);
        auto *buttonPtr = button.get();
        addAndMakeVisible(*button);
        button->onClick = [this, buttonPtr] {
            if (!buttonPtr->isEnabled())
                return;

            if (onSelect)
                onSelect(buttonPtr->mode);

            if (auto *callout = findParentComponentOfClass<juce::CallOutBox>())
                callout->dismiss();
        };
        buttons[index] = std::move(button);
    }
}

SignalModeSelectionPopupContent::~SignalModeSelectionPopupContent() {
    if (onDismiss)
        onDismiss();
}

void SignalModeSelectionPopupContent::paint(juce::Graphics &g) {
    Ui::paintPopupShell(g, getLocalBounds().toFloat(), theme);
}

void SignalModeSelectionPopupContent::setAvailability(
    const std::function<bool(Analyzer::SignalMode)> &isAvailable) {
    for (size_t index = 0; index < buttons.size(); ++index) {
        const auto &option = Ui::signalSlotOptions[index];
        auto &button = buttons[index];
        const auto visible = option.source == currentSource;
        button->setVisible(visible);
        button->setEnabled(visible && isAvailable(option.mode));
    }
}

void SignalModeSelectionPopupContent::resized() {
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

int SignalModeSelectionPopupContent::getPreferredHeight() const {
    const auto &popupMetrics = theme.metrics.popup;
    const auto rowCount = Ui::getVisibleSignalSlotOptionCount(currentSource, true);
    return static_cast<int>(popupMetrics.padding * 2
                            + popupMetrics.rowHeight * static_cast<float>(rowCount)
                            + popupMetrics.rowGap * static_cast<float>(juce::jmax<size_t>(0, rowCount - 1)));
}

int SignalModeSelectionPopupContent::getPreferredWidth() const {
    return 132;
}
