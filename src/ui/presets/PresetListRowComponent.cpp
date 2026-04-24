#include "PresetListRowComponent.h"

#include "ui/theme/UiIcons.h"
#include "ui/theme/UiRasterAssets.h"

namespace {
    constexpr int screenTransparentInset = 3;
}

PresetListRowComponent::PresetListRowComponent(const Ui::Theme& themeToUse,
                                               PluginPresets::PresetDescriptor descriptorToUse,
                                               const bool isCurrentPresetToUse,
                                               Callbacks callbacksToUse)
    : theme(themeToUse),
      descriptor(std::move(descriptorToUse)),
      callbacks(std::move(callbacksToUse)),
      isCurrentPreset(isCurrentPresetToUse) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void PresetListRowComponent::setCallbacks(Callbacks callbacksToUse) {
    callbacks = std::move(callbacksToUse);
}

void PresetListRowComponent::setKeyboardSelected(const bool shouldBeSelected) {
    if (isKeyboardSelected == shouldBeSelected)
        return;

    isKeyboardSelected = shouldBeSelected;
    repaint();
}

void PresetListRowComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    const auto& popupMetrics = theme.metrics.popup;
    const auto isHighlighted = isHovered || isKeyboardSelected;

    if (isCurrentPreset) {
        const auto& screen = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::screen);
        Ui::drawAssetWithin(g,
                            screen,
                            getLocalBounds(),
                            {screenTransparentInset,
                             screenTransparentInset,
                             screen.getWidth() - screenTransparentInset * 2,
                             screen.getHeight() - screenTransparentInset * 2});
        if (isKeyboardSelected) {
            g.setColour(theme.sectionDividerHighlight.withMultipliedAlpha(theme.metrics.presetPopup.currentRowOutlineAlpha));
            g.drawRoundedRectangle(bounds.reduced(popupMetrics.rowOutlineInset),
                                   popupMetrics.rowCornerRadius,
                                   popupMetrics.rowOutlineThickness);
        }
    } else {
        auto fill = theme.controlSurface.withMultipliedBrightness(theme.metrics.presetPopup.browserRowFillBrightness);
        if (isHighlighted)
            fill = theme.controlSurfaceHover;

        g.setColour(fill);
        g.fillRoundedRectangle(bounds, popupMetrics.rowCornerRadius);
        g.setColour(theme.sectionDividerHighlight.withMultipliedAlpha(popupMetrics.rowOutlineAlpha));
        g.drawRoundedRectangle(bounds.reduced(popupMetrics.rowOutlineInset),
                               popupMetrics.rowCornerRadius,
                               popupMetrics.rowOutlineThickness);
    }

    g.setColour(isCurrentPreset ? theme.hardwareMarkingDark
                                : (descriptor.origin == PluginPresets::PresetOrigin::user
                                       ? theme.hardwareMarkingLight
                                       : theme.axisText.brighter(theme.metrics.presetPopup.factoryPresetTextBrightness)));
    g.setFont(juce::FontOptions(popupMetrics.rowTextFontHeight, juce::Font::bold));
    auto textBounds = getLocalBounds().reduced(theme.metrics.presetPopup.browserRowTextInset, 0);
    if (!deleteBounds.isEmpty())
        textBounds.setRight(deleteBounds.getX() - theme.metrics.presetPopup.deleteIconInset);
    g.drawText(descriptor.name, textBounds, juce::Justification::centredLeft, false);

    if (!deleteBounds.isEmpty()) {
        const auto deleteColour = (isCurrentPreset ? theme.hardwareMarkingDark : theme.axisText)
            .withMultipliedAlpha(isDeleteHovered || isKeyboardSelected ? 1.0f
                                                                       : theme.metrics.presetPopup.deleteIconIdleAlpha);
        g.setColour(deleteColour);
        Ui::drawIcon(g, Ui::IconId::cancel, deleteBounds.toFloat(), deleteColour);
    }
}

void PresetListRowComponent::resized() {
    deleteBounds = {};
    if (!descriptor.isDeletable)
        return;

    const auto iconSize = theme.metrics.presetPopup.deleteIconSize;
    deleteBounds = juce::Rectangle<int>(iconSize, iconSize)
                       .withRightX(getWidth() - theme.metrics.presetPopup.deleteIconInset)
                       .withCentre({getWidth() - theme.metrics.presetPopup.deleteIconInset - iconSize / 2, getHeight() / 2});
}

void PresetListRowComponent::mouseMove(const juce::MouseEvent& event) {
    const auto newDeleteHover = deleteBounds.contains(event.getPosition());
    if (isHovered && isDeleteHovered == newDeleteHover)
        return;

    isHovered = true;
    isDeleteHovered = newDeleteHover;
    repaint();
}

void PresetListRowComponent::mouseExit(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);

    if (!isHovered && !isDeleteHovered)
        return;

    isHovered = false;
    isDeleteHovered = false;
    repaint();
}

void PresetListRowComponent::mouseUp(const juce::MouseEvent& event) {
    if (event.mouseWasDraggedSinceMouseDown())
        return;

    if (descriptor.isDeletable && deleteBounds.contains(event.getPosition())) {
        if (callbacks.onDeleteRequested)
            callbacks.onDeleteRequested(descriptor, deleteBounds);
        return;
    }

    if (callbacks.onLoadRequested)
        callbacks.onLoadRequested(descriptor.id);
}
