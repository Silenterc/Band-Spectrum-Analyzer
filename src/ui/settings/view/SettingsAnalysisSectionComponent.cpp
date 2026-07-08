#include "ui/settings/view/SettingsAnalysisSectionComponent.h"

namespace {
    constexpr std::array<const char*, 4> bandModeLabels{
        "1/3 Oct",
        "1/4 Oct",
        "1/6 Oct",
        "1/12 Oct"
    };
}

SettingsAnalysisSectionComponent::SettingsAnalysisSectionComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      frame(themeToUse, "ANALYSIS") {
    addAndMakeVisible(frame);
    createBandModeButtons();
    selectBandModeButton(selectedBandModeIndex);
}

void SettingsAnalysisSectionComponent::paint(juce::Graphics& g) {
    g.setColour(theme.hardwareMarkingLight);
    g.setFont(juce::FontOptions(theme.metrics.rectanglePad.labelFontHeight).withStyle("Bold"));
    g.drawText("BAND MODE", bandModeLabelBounds, juce::Justification::centred, false);
}

void SettingsAnalysisSectionComponent::resized() {
    frame.setBounds(getLocalBounds());
    layoutBandModeButtons(getLocalBounds());
}

void SettingsAnalysisSectionComponent::createBandModeButtons() {
    for (auto index = 0; index < static_cast<int>(bandModeButtons.size()); ++index) {
        auto button = std::make_unique<RasterRectanglePadButton>(theme, bandModeLabels[static_cast<size_t>(index)]);
        button->onClick = [this, index] {
            selectBandModeButton(index);
        };
        addAndMakeVisible(*button);
        bandModeButtons[static_cast<size_t>(index)] = std::move(button);
    }
}

void SettingsAnalysisSectionComponent::selectBandModeButton(const int selectedIndex) {
    selectedBandModeIndex = juce::jlimit(0, static_cast<int>(bandModeButtons.size()) - 1, selectedIndex);
    for (auto index = 0; index < static_cast<int>(bandModeButtons.size()); ++index) {
        if (bandModeButtons[static_cast<size_t>(index)] != nullptr)
            bandModeButtons[static_cast<size_t>(index)]->setActive(index == selectedBandModeIndex);
    }
}

void SettingsAnalysisSectionComponent::layoutBandModeButtons(const juce::Rectangle<int> bounds) {
    const auto& metrics = theme.metrics.settingsAnalysisSection;
    const auto buttonBounds = bandModeButtons.front()->getPreferredBounds();
    const auto buttonGap = metrics.buttonGap;
    const auto totalButtonWidth = buttonBounds.getWidth() * static_cast<int>(bandModeButtons.size())
                                  + buttonGap * (static_cast<int>(bandModeButtons.size()) - 1);
    const auto startX = bounds.getCentreX() - totalButtonWidth / 2 + metrics.buttonGroupOffsetX;
    const auto buttonY = bounds.getCentreY() - buttonBounds.getHeight() / 2 + metrics.buttonOffsetY;
    bandModeLabelBounds = juce::Rectangle<int>(metrics.labelWidth, buttonBounds.getHeight())
                              .withCentre({startX - metrics.labelRightToButtonLeftGap - metrics.labelWidth / 2,
                                           buttonY + buttonBounds.getHeight() / 2});

    for (auto index = 0; index < static_cast<int>(bandModeButtons.size()); ++index) {
        if (bandModeButtons[static_cast<size_t>(index)] == nullptr)
            continue;

        bandModeButtons[static_cast<size_t>(index)]->setBounds(
            buttonBounds.withPosition(startX + index * (buttonBounds.getWidth() + buttonGap), buttonY));
    }
}
