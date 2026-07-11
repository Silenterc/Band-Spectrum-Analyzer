#include "ui/settings/view/SettingsAnalysisSectionComponent.h"

SettingsAnalysisSectionComponent::SettingsAnalysisSectionComponent(AnalyzerSettingsActions& settingsActionsToUse,
                                                                   const Ui::Theme& themeToUse)
    : settingsActions(settingsActionsToUse),
      theme(themeToUse),
      frame(themeToUse, "BAND MODE") {
    addAndMakeVisible(frame);
    createBandModeButtons();
    selectBandModeButton(selectedBandModeIndex);
}

void SettingsAnalysisSectionComponent::applySnapshot(const Ui::AnalyzerUiSnapshot& snapshot) {
    selectBandModeButton(Shared::indexForValue(snapshot.bandMode, Shared::bandModeChoices));
}

void SettingsAnalysisSectionComponent::resized() {
    frame.setBounds(getLocalBounds());
    layoutBandModeButtons(getLocalBounds());
}

void SettingsAnalysisSectionComponent::createBandModeButtons() {
    for (auto index = 0; index < static_cast<int>(bandModeButtons.size()); ++index) {
        auto button = std::make_unique<RasterRectanglePadButton>(
            theme, Shared::bandModeChoices[static_cast<size_t>(index)].label);
        button->onClick = [this, index] {
            selectBandModeButton(index);
            settingsActions.setBandMode(Shared::bandModeChoices[static_cast<size_t>(index)].value);
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
    const auto contentBounds = bounds.reduced(metrics.horizontalInset, 0);
    const auto buttonBounds = bandModeButtons.front()->getPreferredBounds();
    const auto visualCenterOffset = bandModeButtons.front()->getVisualCenterOffset();
    const auto buttonGap = metrics.buttonGap;
    const auto totalButtonWidth = buttonBounds.getWidth() * static_cast<int>(bandModeButtons.size())
                                  + buttonGap * (static_cast<int>(bandModeButtons.size()) - 1);
    const auto frameCentreY =
        SettingsSectionFrameComponent::getBorderBounds(bounds, theme.metrics.settingsSectionFrame).getCentreY();
    const auto startX = juce::roundToInt(static_cast<float>(contentBounds.getCentreX() - totalButtonWidth / 2)
                                         - visualCenterOffset.x);
    const auto buttonY = juce::roundToInt(frameCentreY
                                          - static_cast<float>(buttonBounds.getHeight()) * 0.5f
                                          - visualCenterOffset.y);

    for (auto index = 0; index < static_cast<int>(bandModeButtons.size()); ++index) {
        if (bandModeButtons[static_cast<size_t>(index)] == nullptr)
            continue;

        bandModeButtons[static_cast<size_t>(index)]->setBounds(
            buttonBounds.withPosition(startX + index * (buttonBounds.getWidth() + buttonGap), buttonY));
    }
}
