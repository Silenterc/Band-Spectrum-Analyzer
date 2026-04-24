#include "SignalSlotActionButton.h"

#include "ui/theme/UiButtonDrawing.h"

SignalSlotActionButton::SignalSlotActionButton(const Ui::Theme &themeToUse)
    : juce::Button({}),
      theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTriggeredOnMouseDown(false);
}

void SignalSlotActionButton::setStyle(const Style &styleToUse) {
    style = styleToUse;
    repaint();
}

void SignalSlotActionButton::paintButton(juce::Graphics &g, const bool isMouseOverButton, const bool isButtonDown) {
    juce::ignoreUnused(isButtonDown);
    const auto &slotMetrics = theme.metrics.slot;
    const auto bounds = getLocalBounds().toFloat();
    const auto fill = isMouseOverButton ? style.hoverFill : style.fill;

    if (style.content == Content::cancel) {
        if (style.drawsBackground) {
            g.setColour(fill);
            g.fillRoundedRectangle(bounds, slotMetrics.buttonCornerRadius);
        }
        Ui::drawIcon(g,
                     Ui::IconId::cancel,
                     bounds.reduced(slotMetrics.cancelIconInset),
                     style.foreground);
        return;
    }

    if (style.content == Content::power) {
        if (style.drawsBackground) {
            g.setColour(fill);
            g.fillRoundedRectangle(bounds, slotMetrics.buttonCornerRadius);
        }
        Ui::drawIcon(g, Ui::IconId::power, bounds.reduced(slotMetrics.powerIconInset), style.foreground);
        return;
    }

    if (style.content == Content::snowflake) {
        Ui::IconActionButtonStyle snowflakeStyle;
        snowflakeStyle.fill = fill;
        snowflakeStyle.icon = style.foreground;
        Ui::drawSnowflakeActionButton(g, bounds, theme, snowflakeStyle, slotMetrics.snowflakeIconInset);
        return;
    }

    if (style.drawsBackground) {
        g.setColour(fill);
        g.fillRoundedRectangle(bounds, slotMetrics.buttonCornerRadius);
    }
    g.setColour(style.foreground);
    g.setFont(style.fontHeight);
    g.drawText(style.text, getLocalBounds(), juce::Justification::centred);
}
