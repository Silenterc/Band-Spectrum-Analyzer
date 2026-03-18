#include "SignalSlotActionButton.h"

#include "../../UiButtonDrawing.h"

SignalSlotActionButton::SignalSlotActionButton(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void SignalSlotActionButton::setStyle(const Style &styleToUse) {
    style = styleToUse;
    repaint();
}

void SignalSlotActionButton::paint(juce::Graphics &g) {
    const auto bounds = getLocalBounds().toFloat();
    const auto fill = hovered ? style.hoverFill : style.fill;

    if (style.content == Content::snowflake) {
        Ui::SnowflakeButtonStyle snowflakeStyle;
        snowflakeStyle.fill = fill;
        snowflakeStyle.icon = style.foreground;
        Ui::drawSnowflakeActionButton(g, bounds, theme, snowflakeStyle, 4.0f);
        return;
    }

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, theme.metrics.slot.buttonCornerRadius);
    g.setColour(style.foreground);
    g.setFont(style.fontHeight);
    g.drawText(style.text, getLocalBounds(), juce::Justification::centred);
}

void SignalSlotActionButton::mouseEnter(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hovered = true;
    repaint();
}

void SignalSlotActionButton::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hovered = false;
    repaint();
}

void SignalSlotActionButton::mouseUp(const juce::MouseEvent &event) {
    if (!event.mouseWasDraggedSinceMouseDown() && onClick)
        onClick();
}
