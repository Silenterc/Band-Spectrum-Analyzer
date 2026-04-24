#include "PresetSelectorButton.h"

#include "ui/theme/UiRasterAssets.h"

PresetSelectorButton::PresetSelectorButton(const Ui::Theme& themeToUse)
    : juce::Button({}),
      theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTriggeredOnMouseDown(false);
    setWantsKeyboardFocus(true);
}

void PresetSelectorButton::setDisplayText(juce::String newDisplayText) {
    if (displayText == newDisplayText)
        return;

    displayText = std::move(newDisplayText);
    repaint();
}

void PresetSelectorButton::paintButton(juce::Graphics& g, const bool isMouseOverButton, const bool isButtonDown) {
    juce::ignoreUnused(isMouseOverButton, isButtonDown);

    if (cachedBackgroundImage.isValid())
        g.drawImageAt(cachedBackgroundImage, 0, 0);

    g.setColour(theme.hardwareMarkingLight);
    g.setFont(juce::FontOptions(theme.metrics.presetHeader.labelFontHeight).withStyle("Bold"));
    g.drawText(displayText, getLocalBounds(), juce::Justification::centred, false);

    if (hasKeyboardFocus(true)) {
        const auto& metrics = theme.metrics.presetHeader;
        g.setColour(theme.sectionDividerHighlight.withMultipliedAlpha(metrics.selectorFocusOutlineAlpha));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(metrics.selectorFocusOutlineInset),
                               metrics.selectorFocusCornerRadius,
                               metrics.selectorFocusOutlineThickness);
    }
}

void PresetSelectorButton::resized() {
    if (getWidth() <= 0 || getHeight() <= 0) {
        cachedBackgroundImage = {};
        return;
    }

    cachedBackgroundImage = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::textBox).rescaled(
        getWidth(),
        getHeight(),
        juce::Graphics::highResamplingQuality);
}

void PresetSelectorButton::buttonStateChanged() {
    juce::Button::buttonStateChanged();

    const auto isCurrentlyDown = isDown();
    if (!wasDown && isCurrentlyDown && onPressed)
        onPressed();

    wasDown = isCurrentlyDown;
}

bool PresetSelectorButton::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::downKey) {
        if (onDownArrow)
            onDownArrow();
        return true;
    }

    return juce::Button::keyPressed(key);
}
