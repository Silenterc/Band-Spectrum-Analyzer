#include "ui/settings/view/SettingsUiSectionComponent.h"

SettingsUiSectionComponent::SettingsUiSectionComponent(EditorPresentationActions& presentationActionsToUse,
                                                       const Ui::Theme& themeToUse)
    : presentationActions(presentationActionsToUse),
      theme(themeToUse),
      frame(themeToUse, "UI SCALE") {
    addAndMakeVisible(frame);
    createScaleButtons();
    selectScaleButton(selectedScaleIndex);
}

void SettingsUiSectionComponent::applyPresentationState(const Ui::EditorPresentationState& state) {
    selectScaleButton(Shared::indexForValue(state.scale, Shared::uiScaleChoices));
}

void SettingsUiSectionComponent::resized() {
    frame.setBounds(getLocalBounds());
    layoutScaleButtons(getLocalBounds());
}

void SettingsUiSectionComponent::createScaleButtons() {
    for (auto index = 0; index < static_cast<int>(scaleButtons.size()); ++index) {
        auto button = std::make_unique<RasterRectanglePadButton>(
            theme, Shared::uiScaleChoices[static_cast<std::size_t>(index)].label);
        button->onClick = [this, index] {
            selectScaleButton(index);
            presentationActions.setUiScalePreset(Shared::uiScaleChoices[static_cast<std::size_t>(index)].value);
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
    const auto visualCenterOffset = scaleButtons.front()->getVisualCenterOffset();
    const auto totalButtonWidth = buttonBounds.getWidth() * static_cast<int>(scaleButtons.size())
                                  + metrics.buttonGap * (static_cast<int>(scaleButtons.size()) - 1);
    const auto frameCentreY =
        SettingsSectionFrameComponent::getBorderBounds(bounds, theme.metrics.settingsSectionFrame).getCentreY();
    const auto buttonStartX = juce::roundToInt(static_cast<float>(contentBounds.getCentreX() - totalButtonWidth / 2)
                                               - visualCenterOffset.x);
    const auto buttonY = juce::roundToInt(frameCentreY
                                          - static_cast<float>(buttonBounds.getHeight()) * 0.5f
                                          - visualCenterOffset.y);

    for (auto index = 0; index < static_cast<int>(scaleButtons.size()); ++index) {
        if (scaleButtons[static_cast<std::size_t>(index)] == nullptr)
            continue;

        scaleButtons[static_cast<std::size_t>(index)]->setBounds(
            buttonBounds.withPosition(buttonStartX + index * (buttonBounds.getWidth() + metrics.buttonGap),
                                      buttonY));
    }
}
