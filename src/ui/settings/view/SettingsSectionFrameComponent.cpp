#include "ui/settings/view/SettingsSectionFrameComponent.h"

SettingsSectionFrameComponent::SettingsSectionFrameComponent(const Ui::Theme& themeToUse,
                                                             juce::String titleText)
    : theme(themeToUse),
      title(std::move(titleText)) {
    setInterceptsMouseClicks(false, false);
}

void SettingsSectionFrameComponent::paint(juce::Graphics& g) {
    const auto& metrics = theme.metrics.settingsSectionFrame;
    auto bounds = getLocalBounds().toFloat().reduced(metrics.strokeWidth * 0.5f);
    if (bounds.isEmpty())
        return;

    const auto borderColour = theme.hardwareMarkingDark
                                  .interpolatedWith(theme.hardwareMarkingActiveLight, 0.42f)
                                  .withAlpha(metrics.borderAlpha);
    const auto font = juce::FontOptions(metrics.titleFontHeight).withStyle("Bold");
    g.setFont(font);

    const auto titleWidth = title.isEmpty()
                                ? 0
                                : juce::roundToInt(juce::TextLayout::getStringWidth(g.getCurrentFont(), title))
                                  + metrics.titleHorizontalPadding * 2;
    const auto frameTop = static_cast<float>(metrics.titleGapHeight) * 0.5f;
    bounds.setY(bounds.getY() + frameTop);
    bounds.setHeight(bounds.getHeight() - frameTop);

    const auto x = bounds.getX();
    const auto y = bounds.getY();
    const auto right = bounds.getRight();
    const auto bottom = bounds.getBottom();
    const auto radius = juce::jmin(metrics.cornerRadius, juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f);
    const auto centreX = bounds.getCentreX();
    const auto titleGapLeft = centreX - static_cast<float>(titleWidth) * 0.5f;
    const auto titleGapRight = centreX + static_cast<float>(titleWidth) * 0.5f;
    const auto topLeftEnd = juce::jlimit(x + radius, right - radius, titleGapLeft);
    const auto topRightStart = juce::jlimit(x + radius, right - radius, titleGapRight);

    juce::Path framePath;
    framePath.startNewSubPath(x + radius, y);
    framePath.lineTo(topLeftEnd, y);
    framePath.startNewSubPath(topRightStart, y);
    framePath.lineTo(right - radius, y);
    framePath.quadraticTo(right, y, right, y + radius);
    framePath.lineTo(right, bottom - radius);
    framePath.quadraticTo(right, bottom, right - radius, bottom);
    framePath.lineTo(x + radius, bottom);
    framePath.quadraticTo(x, bottom, x, bottom - radius);
    framePath.lineTo(x, y + radius);
    framePath.quadraticTo(x, y, x + radius, y);

    g.setColour(borderColour);
    g.strokePath(framePath, juce::PathStrokeType(metrics.strokeWidth));

    if (title.isEmpty())
        return;

    const auto titleBounds = juce::Rectangle<int>(titleWidth, metrics.titleGapHeight)
                                 .withCentre({getLocalBounds().getCentreX(), juce::roundToInt(y)});

    g.setColour(borderColour.withAlpha(metrics.titleAlpha));
    g.drawText(title, titleBounds, juce::Justification::centred, false);
}

bool SettingsSectionFrameComponent::hitTest(const int x, const int y) {
    juce::ignoreUnused(x, y);
    return false;
}

void SettingsSectionFrameComponent::setTitle(juce::String newTitle) {
    if (title == newTitle)
        return;

    title = std::move(newTitle);
    repaint();
}
