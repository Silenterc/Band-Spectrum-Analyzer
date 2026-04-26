#include "SignalColourPopupContent.h"

#include "ui/theme/PopupChrome.h"

namespace {
    int getColourRowCount(const int itemCount, const int columns) {
        if (columns <= 0)
            return 0;

        return (itemCount + columns - 1) / columns;
    }

    class SignalColourButton final : public juce::Button {
    public:
        SignalColourButton(const Ui::Theme &themeToUse, const juce::Colour colourToUse, const bool selectedToUse)
            : juce::Button({}), theme(themeToUse), colour(colourToUse), selected(selectedToUse) {
        }

        void paintButton(juce::Graphics &g, bool isMouseOverButton, bool) override {
            const auto bounds = getLocalBounds().toFloat().reduced(theme.metrics.popup.swatchInset);
            const auto &popupMetrics = theme.metrics.popup;

            g.setColour(colour.withMultipliedAlpha(isEnabled() ? 1.0f : 0.28f));
            g.fillEllipse(bounds);

            if (isMouseOverButton) {
                g.setColour(theme.sectionDividerHighlight.withMultipliedAlpha(popupMetrics.swatchHoverAlpha));
                g.fillEllipse(bounds.reduced(popupMetrics.swatchHoverInset));
            }

            g.setColour(selected ? theme.hardwareMarkingLight
                                 : theme.sectionDividerHighlight.withMultipliedAlpha(popupMetrics.swatchOutlineAlpha));
            g.drawEllipse(bounds, selected ? popupMetrics.swatchSelectedOutlineThickness : popupMetrics.swatchOutlineThickness);

            if (!isEnabled()) {
                g.setColour(theme.axisText.withMultipliedAlpha(0.18f));
                g.drawLine(bounds.getX() + popupMetrics.swatchDisabledSlashInset,
                           bounds.getBottom() - popupMetrics.swatchDisabledSlashInset,
                           bounds.getRight() - popupMetrics.swatchDisabledSlashInset,
                           bounds.getY() + popupMetrics.swatchDisabledSlashInset,
                           popupMetrics.swatchDisabledSlashThickness);
            }
        }

    private:
        const Ui::Theme &theme;
        juce::Colour colour;
        bool selected = false;
    };
}

SignalColourPopupContent::SignalColourPopupContent(const Ui::Theme &themeToUse,
                                                   std::function<void(int)> onSelectToUse,
    std::function<void()> onCloseRequestedToUse,
    std::function<void()> onDismissedToUse)
    : theme(themeToUse),
      onSelect(std::move(onSelectToUse)),
      onCloseRequested(std::move(onCloseRequestedToUse)),
      onDismissed(std::move(onDismissedToUse)) {
    jassert(onSelect != nullptr);
    jassert(onCloseRequested != nullptr);
    jassert(onDismissed != nullptr);
}

SignalColourPopupContent::~SignalColourPopupContent() {
    onDismissed();
}

void SignalColourPopupContent::paint(juce::Graphics &g) {
    Ui::paintPopupShell(g, getLocalBounds().toFloat(), theme);
}

void SignalColourPopupContent::addColourButton(const juce::Colour colour,
                                               const bool selected,
                                               const bool enabled,
                                               const int colourIndex) {
    auto button = std::make_unique<SignalColourButton>(theme, colour, selected);
    button->setEnabled(enabled);
    button->onClick = [this, colourIndex] {
        onSelect(colourIndex);
        onCloseRequested();
    };
    addAndMakeVisible(*button);
    colourButtons.push_back(std::move(button));
}

void SignalColourPopupContent::resized() {
    const auto &popupMetrics = theme.metrics.popup;
    auto bounds = getLocalBounds().reduced(static_cast<int>(popupMetrics.padding));
    const auto columns = popupMetrics.colourColumns;
    const auto itemSize = static_cast<int>(popupMetrics.swatchSize);
    const auto gap = static_cast<int>(popupMetrics.colourGap);

    auto x = bounds.getX();
    auto y = bounds.getY();
    for (size_t index = 0; index < colourButtons.size(); ++index) {
        colourButtons[index]->setBounds(x, y, itemSize, itemSize);
        x += itemSize + gap;
        if ((index + 1) % static_cast<size_t>(columns) == 0) {
            x = bounds.getX();
            y += itemSize + gap;
        }
    }
}

int SignalColourPopupContent::getPreferredWidth() const {
    const auto &popupMetrics = theme.metrics.popup;
    return static_cast<int>(popupMetrics.padding * 2
                            + popupMetrics.swatchSize * static_cast<float>(popupMetrics.colourColumns)
                            + popupMetrics.colourGap * static_cast<float>(popupMetrics.colourColumns - 1));
}

int SignalColourPopupContent::getPreferredHeight() const {
    const auto &popupMetrics = theme.metrics.popup;
    const auto rows = getColourRowCount(static_cast<int>(colourButtons.size()), popupMetrics.colourColumns);
    return static_cast<int>(popupMetrics.padding * 2
                            + popupMetrics.swatchSize * static_cast<float>(rows)
                            + popupMetrics.colourGap * static_cast<float>(juce::jmax(0, rows - 1)));
}
