#include "PopupActionButton.h"

#include "ui/theme/PopupChrome.h"

PopupActionButton::PopupActionButton(const Ui::Theme& themeToUse)
    : juce::Button({}),
      theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTriggeredOnMouseDown(false);
}

void PopupActionButton::setLabel(juce::String newLabel) {
    if (label == newLabel)
        return;

    label = std::move(newLabel);
    repaint();
}

void PopupActionButton::setStyle(const Style newStyle) {
    if (style == newStyle)
        return;

    style = newStyle;
    repaint();
}

void PopupActionButton::paintButton(juce::Graphics& g, const bool isMouseOverButton, const bool isButtonDown) {
    auto bounds = getLocalBounds().toFloat();
    const auto& popupMetrics = theme.metrics.popup;
    const auto isPrimary = style == Style::primary;

    auto fill = isPrimary
                    ? theme.controlSurfaceHover.brighter(popupMetrics.buttonPrimaryFillBrightness)
                    : theme.controlSurface.withMultipliedBrightness(popupMetrics.buttonSecondaryFillBrightness);
    if (isMouseOverButton)
        fill = fill.brighter(popupMetrics.buttonHoverBrightness);
    if (isButtonDown)
        fill = fill.darker(popupMetrics.buttonPressedDarkness);
    if (!isEnabled())
        fill = fill.withMultipliedAlpha(popupMetrics.buttonDisabledFillAlpha);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, popupMetrics.rowCornerRadius);
    g.setColour(theme.sectionDividerHighlight.withMultipliedAlpha(popupMetrics.shellBorderAlpha));
    g.drawRoundedRectangle(bounds.reduced(popupMetrics.rowOutlineInset),
                           popupMetrics.rowCornerRadius,
                           popupMetrics.rowOutlineThickness);

    g.setColour(isPrimary ? theme.hardwareMarkingLight
                          : theme.axisText.brighter(popupMetrics.buttonSecondaryTextBrightness));
    if (!isEnabled())
        g.setOpacity(popupMetrics.buttonDisabledTextOpacity);
    g.setFont(juce::FontOptions(theme.metrics.presetPopup.titleFontHeight, juce::Font::bold));
    g.drawText(label, getLocalBounds(), juce::Justification::centred, false);
}
