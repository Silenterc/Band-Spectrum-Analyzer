#include "PresetHeaderComponent.h"

PresetHeaderComponent::PresetHeaderComponent(PresetUiSnapshotSource& presetUiSnapshotSourceToUse,
                                             PresetActions& presetActionsToUse,
                                             const Ui::Theme& themeToUse)
    : theme(themeToUse),
      controlsComponent(presetUiSnapshotSourceToUse, presetActionsToUse, themeToUse) {
    addAndMakeVisible(controlsComponent);
    setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);
}

void PresetHeaderComponent::paint(juce::Graphics& g) {
    BrandLogoComponent::drawLogo(g, logoBounds);
}

void PresetHeaderComponent::resized() {
    const auto& metrics = theme.metrics.presetHeader;
    auto contentBounds = getLocalBounds();
    contentBounds.removeFromLeft(metrics.logoLeftInset);
    logoBounds = getLogoBounds(contentBounds);

    auto controlsBounds = juce::Rectangle<int>(controlsComponent.getPreferredWidth(), getHeight())
                              .withY(getLocalBounds().getY())
                              .withX(logoBounds.isEmpty() ? contentBounds.getX() : logoBounds.getRight() + metrics.logoGap);
    controlsComponent.setBounds(controlsBounds);
}

juce::Rectangle<int> PresetHeaderComponent::getLogoBounds(const juce::Rectangle<int> availableBounds) const {
    return BrandLogoComponent::getLogoBounds(availableBounds, theme);
}
