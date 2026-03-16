#include "SignalColourPopupContent.h"

namespace {
    class SignalColourButton final : public juce::Button {
    public:
        SignalColourButton(const juce::Colour colourToUse, const bool selectedToUse)
            : juce::Button({}), colour(colourToUse), selected(selectedToUse) {
        }

        void paintButton(juce::Graphics &g, bool isMouseOverButton, bool) override {
            auto bounds = getLocalBounds().toFloat().reduced(4.0f);

            g.setColour(colour.withMultipliedAlpha(isEnabled() ? 1.0f : 0.28f));
            g.fillEllipse(bounds);

            if (isMouseOverButton) {
                g.setColour(juce::Colours::white.withAlpha(0.12f));
                g.fillEllipse(bounds.reduced(2.0f));
            }

            g.setColour(selected ? juce::Colours::white : juce::Colours::white.withAlpha(0.14f));
            g.drawEllipse(bounds, selected ? 2.0f : 1.0f);

            if (!isEnabled()) {
                g.setColour(juce::Colours::white.withAlpha(0.18f));
                g.drawLine(bounds.getX() + 5.0f, bounds.getBottom() - 5.0f,
                           bounds.getRight() - 5.0f, bounds.getY() + 5.0f, 1.5f);
            }
        }

    private:
        juce::Colour colour;
        bool selected = false;
    };
}

SignalColourPopupContent::SignalColourPopupContent(const Ui::Theme &themeToUse,
                                                   std::function<void(int)> onSelectToUse,
                                                   std::function<void()> onDismissToUse)
    : onSelect(std::move(onSelectToUse)),
      theme(themeToUse),
      onDismiss(std::move(onDismissToUse)) {
}

SignalColourPopupContent::~SignalColourPopupContent() {
    if (onDismiss)
        onDismiss();
}

void SignalColourPopupContent::addColourButton(const juce::Colour colour,
                                               const bool selected,
                                               const bool enabled,
                                               const int colourIndex) {
    auto button = std::make_unique<SignalColourButton>(colour, selected);
    button->setEnabled(enabled);
    button->onClick = [this, colourIndex] {
        if (!onSelect)
            return;

        onSelect(colourIndex);

        if (auto *callout = findParentComponentOfClass<juce::CallOutBox>())
            callout->dismiss();
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
    constexpr int rows = 2;
    return static_cast<int>(popupMetrics.padding * 2
                            + popupMetrics.swatchSize * static_cast<float>(rows)
                            + popupMetrics.colourGap * static_cast<float>(rows - 1));
}
