#include "PresetPopupStatusComponent.h"

PresetPopupStatusComponent::PresetPopupStatusComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse) {
    setInterceptsMouseClicks(false, false);
}

void PresetPopupStatusComponent::setMessage(juce::String newMessage) {
    if (message == newMessage)
        return;

    message = std::move(newMessage);
    repaint();
}

bool PresetPopupStatusComponent::hasMessage() const {
    return message.isNotEmpty();
}

int PresetPopupStatusComponent::getPreferredHeight() const {
    return hasMessage() ? theme.metrics.presetPopup.statusMinHeight : 0;
}

void PresetPopupStatusComponent::paint(juce::Graphics& g) {
    if (message.isEmpty())
        return;

    g.setColour(theme.presetPopupStatusText);
    g.setFont(juce::FontOptions(theme.metrics.presetPopup.statusFontHeight, juce::Font::bold));
    g.drawFittedText(message, getLocalBounds(), juce::Justification::centred, 2);
}
