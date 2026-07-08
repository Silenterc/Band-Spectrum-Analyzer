#include "ui/settings/view/SettingsUiSectionComponent.h"

namespace {
    constexpr std::array<const char*, 3> scaleLabels{
        "1x",
        "1.5x",
        "2x"
    };
}

SettingsUiSectionComponent::SettingsUiSectionComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      frame(themeToUse, "UI") {
    addAndMakeVisible(frame);
    createScaleButtons();
    selectScaleButton(selectedScaleIndex);
}

void SettingsUiSectionComponent::paint(juce::Graphics& g) {
    g.setColour(theme.hardwareMarkingLight);
    g.setFont(juce::FontOptions(theme.metrics.rectanglePad.labelFontHeight).withStyle("Bold"));
    g.drawText("UI SCALE", scaleLabelBounds, juce::Justification::centredRight, false);
}

void SettingsUiSectionComponent::resized() {
    frame.setBounds(getLocalBounds());
    layoutScaleButtons(getLocalBounds());
}

void SettingsUiSectionComponent::createScaleButtons() {
    for (auto index = 0; index < static_cast<int>(scaleButtons.size()); ++index) {
        auto button = std::make_unique<RasterRectanglePadButton>(theme, scaleLabels[static_cast<std::size_t>(index)]);
        button->onClick = [this, index] {
            selectScaleButton(index);
        };
        addAndMakeVisible(*button);
        scaleButtons[static_cast<std::size_t>(index)] = std::move(button);
    }
}

void SettingsUiSectionComponent::selectScaleButton(const int selectedIndex) {
    selectedScaleIndex = juce::jlimit(0, static_cast<int>(scaleButtons.size()) - 1, selectedIndex);
    for (auto index = 0; index < static_cast<int>(scaleButtons.size()); ++index) {
        if (scaleButtons[static_cast<std::size_t>(index)] != nullptr)
            scaleButtons[static_cast<std::size_t>(index)]->setActive(index == selectedScaleIndex);
    }
}

void SettingsUiSectionComponent::layoutScaleButtons(const juce::Rectangle<int> bounds) {
    const auto& metrics = theme.metrics.settingsUiSection;
    const auto contentBounds = bounds.reduced(metrics.horizontalInset, 0);
    const auto buttonBounds = scaleButtons.front()->getPreferredBounds();
    const auto totalButtonWidth = buttonBounds.getWidth() * static_cast<int>(scaleButtons.size())
                                  + metrics.buttonGap * (static_cast<int>(scaleButtons.size()) - 1);
    const auto buttonStartX = contentBounds.getRight() - totalButtonWidth;
    const auto buttonY = bounds.getY() + metrics.contentTopInset;

    const auto availableLabelWidth = juce::jmax(0,
                                                buttonStartX - metrics.labelRightToButtonLeftGap
                                                    - contentBounds.getX());
    scaleLabelBounds = juce::Rectangle<int>(juce::jmin(metrics.labelWidth, availableLabelWidth),
                                            buttonBounds.getHeight())
                           .withRightX(buttonStartX - metrics.labelRightToButtonLeftGap)
                           .withY(buttonY);

    for (auto index = 0; index < static_cast<int>(scaleButtons.size()); ++index) {
        if (scaleButtons[static_cast<std::size_t>(index)] == nullptr)
            continue;

        scaleButtons[static_cast<std::size_t>(index)]->setBounds(
            buttonBounds.withPosition(buttonStartX + index * (buttonBounds.getWidth() + metrics.buttonGap),
                                      buttonY));
    }
}
