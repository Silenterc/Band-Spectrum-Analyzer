#include "AnalyzerMeterControlsComponent.h"

#include <BinaryData.h>

AnalyzerMeterControlsComponent::AnalyzerMeterControlsComponent(AnalyzerUiStateSource &uiStateSourceToUse,
                                                               AnalyzerSettingsActions &settingsActionsToUse,
                                                               const Ui::Theme &themeToUse)
    : uiStateSource(uiStateSourceToUse),
      settingsActions(settingsActionsToUse),
      theme(themeToUse),
      settingsButton(themeToUse, {}),
      peakButton(themeToUse, "Peak"),
      rmsButton(themeToUse, "RMS"),
      holdButton(themeToUse, "Hold"),
      freezeButton(themeToUse, {}) {
    addAndMakeVisible(settingsButton);
    addAndMakeVisible(peakButton);
    addAndMakeVisible(rmsButton);
    addAndMakeVisible(holdButton);
    addAndMakeVisible(freezeButton);

    for (auto *button : { &settingsButton, &peakButton, &rmsButton, &holdButton, &freezeButton })
        button->setWantsKeyboardFocus(false);

    settingsButton.onClick = [] {
        // TODO: Open the settings panel when the skinned settings UI is implemented.
    };

    peakButton.onClick = [this] {
        const auto nextEnabled = !currentState.meterSettings.showPeak;
        currentState.meterSettings.showPeak = nextEnabled;
        peakButton.setActive(nextEnabled);
        settingsActions.setShowPeakEnabled(nextEnabled);
    };
    rmsButton.onClick = [this] {
        const auto nextEnabled = !currentState.meterSettings.showRms;
        currentState.meterSettings.showRms = nextEnabled;
        rmsButton.setActive(nextEnabled);
        settingsActions.setShowRmsEnabled(nextEnabled);
    };
    holdButton.onClick = [this] {
        const auto nextEnabled = !currentState.meterSettings.showHold;
        currentState.meterSettings.showHold = nextEnabled;
        holdButton.setActive(nextEnabled);
        settingsActions.setShowHoldEnabled(nextEnabled);
    };
    freezeButton.onClick = [this] {
        const auto nextFrozen = !currentState.frozen;
        currentState.frozen = nextFrozen;
        freezeButton.setActive(nextFrozen);
        settingsActions.setFreezeEnabled(nextFrozen);
    };

    settingsButton.setTooltip("Open settings");
    settingsButton.setDrawsPad(false);
    settingsButton.setScaleMultiplier(theme.metrics.meterControls.settingsPadScaleMultiplier);
    settingsButton.setOverlayIcon(PadButton::OverlayIcon::settings);
    settingsButton.setOverlayIconScaleMultiplier(theme.metrics.meterControls.settingsIconScaleMultiplier);
    peakButton.setTooltip("Show Peak");
    rmsButton.setTooltip("Show RMS");
    holdButton.setTooltip("Show Peak Hold");
    freezeButton.setTooltip("Freeze analyzer");
    freezeButton.setAssetStyle(PadButton::AssetStyle::freeze);
    freezeButton.setActiveMarkingColour(theme.hardwareMarkingCoolDark);
    freezeButton.setScaleMultiplier(theme.metrics.meterControls.freezePadScaleMultiplier);
    freezeButton.setOverlayIcon(PadButton::OverlayIcon::snowflake);
    freezeButton.setOverlayIconScaleMultiplier(theme.metrics.meterControls.freezeIconScaleMultiplier);

    uiStateSource.addAnalyzerUiStateListener(*this);
    analyzerUiStateChanged(uiStateSource.getAnalyzerUiState());
}

AnalyzerMeterControlsComponent::~AnalyzerMeterControlsComponent() {
    uiStateSource.removeAnalyzerUiStateListener(*this);
}

void AnalyzerMeterControlsComponent::paint(juce::Graphics &g) {
    if (!settingsSeparatorBounds.isEmpty()) {
        juce::ColourGradient gradient(
            theme.sectionDividerShadow.withMultipliedAlpha(0.90f),
            0.0f,
            static_cast<float>(settingsSeparatorBounds.getY()),
            theme.sectionDividerHighlight.withMultipliedAlpha(0.46f),
            0.0f,
            static_cast<float>(settingsSeparatorBounds.getBottom()),
            false);
        gradient.addColour(0.38, theme.sectionDividerShadow.withMultipliedAlpha(0.78f));
        gradient.addColour(0.74, theme.sectionDividerHighlight.withMultipliedAlpha(0.26f));
        g.setGradientFill(gradient);
        g.fillRect(settingsSeparatorBounds);
    }

    if (cachedDecorImage.isValid())
        g.drawImageAt(cachedDecorImage, decorBounds.getX(), decorBounds.getY());
}

void AnalyzerMeterControlsComponent::resized() {
    const auto &metrics = theme.metrics.meterControls;
    const auto contentOffsetX = metrics.padOpticalOffsetX;
    const auto applyContentOffset = [contentOffsetX](juce::Rectangle<int> boundsToOffset) {
        return boundsToOffset.translated(contentOffsetX, 0);
    };
    auto bounds = getLocalBounds().reduced(0, metrics.verticalPadding);
    const auto settingsSize = settingsButton.getPreferredHeight(bounds.getWidth());
    const auto settingsSeparatorY = bounds.getY() + metrics.settingsTopInset + settingsSize + metrics.settingsGap / 2;
    settingsSeparatorBounds = applyContentOffset(juce::Rectangle<int>(
        metrics.settingsSeparatorInset,
        settingsSeparatorY,
        juce::jmax(1, getWidth() - metrics.settingsSeparatorInset * 2),
        metrics.settingsSeparatorThickness));
    const auto settingsArea = juce::Rectangle<int>(
        bounds.getX(),
        bounds.getY(),
        bounds.getWidth(),
        juce::jmax(0, settingsSeparatorBounds.getY() - bounds.getY()));
    const auto settingsBounds = juce::Rectangle<int>(settingsSize, settingsSize)
                                    .withCentre(settingsArea.getCentre());
    settingsButton.setBounds(applyContentOffset(settingsBounds));

    const auto clusterTop = juce::jmax(bounds.getY(), settingsBounds.getBottom() + metrics.settingsGap);
    bounds = bounds.withTrimmedTop(clusterTop - bounds.getY());

    bounds.removeFromBottom(metrics.bottomInset);
    const auto buttonGap = metrics.buttonGap;
    const auto groupGap = metrics.groupGap;
    const auto peakHeight = peakButton.getPreferredHeight(bounds.getWidth());
    const auto rmsHeight = rmsButton.getPreferredHeight(bounds.getWidth());
    const auto holdHeight = holdButton.getPreferredHeight(bounds.getWidth());
    const auto freezeHeight = freezeButton.getPreferredHeight(bounds.getWidth());
    const auto decorHeight = getDecorPreferredHeight(bounds.getWidth());
    const auto topGroupHeight = peakHeight + rmsHeight + holdHeight + buttonGap * 2;
    const auto totalHeight = topGroupHeight + decorHeight + freezeHeight + groupGap * 2;
    const auto topInset = juce::jmax(0, bounds.getHeight() - totalHeight);
    bounds.removeFromTop(topInset);

    peakButton.setBounds(applyContentOffset(bounds.removeFromTop(peakHeight)));
    bounds.removeFromTop(buttonGap);
    rmsButton.setBounds(applyContentOffset(bounds.removeFromTop(rmsHeight)));
    bounds.removeFromTop(buttonGap);
    holdButton.setBounds(applyContentOffset(bounds.removeFromTop(holdHeight)));
    bounds.removeFromTop(groupGap);

    decorBounds = applyContentOffset(bounds.removeFromTop(decorHeight));
    rebuildCachedDecor();

    bounds.removeFromTop(groupGap);
    freezeButton.setBounds(applyContentOffset(bounds.removeFromTop(freezeHeight)));
}

void AnalyzerMeterControlsComponent::analyzerUiStateChanged(const Ui::AnalyzerUiState &state) {
    currentState = state;
    syncButtonStates(currentState);
}

void AnalyzerMeterControlsComponent::syncButtonStates(const Ui::AnalyzerUiState &state) {
    peakButton.setActive(state.meterSettings.showPeak);
    rmsButton.setActive(state.meterSettings.showRms);
    holdButton.setActive(state.meterSettings.showHold);
    freezeButton.setActive(state.frozen);
}

int AnalyzerMeterControlsComponent::getDecorPreferredHeight(const int availableWidth) const {
    const auto& sourceImage = getDecorGridImage();
    const auto rasterScale = theme.metrics.assets.rasterScale;
    const auto logicalWidth = static_cast<float>(sourceImage.getWidth()) / rasterScale;
    const auto logicalHeight = static_cast<float>(sourceImage.getHeight()) / rasterScale;
    const auto widthScale = static_cast<float>(juce::jmax(1, availableWidth)) / logicalWidth;
    const auto scale = juce::jlimit(0.0f, 1.0f, widthScale) * theme.metrics.meterControls.decorScale;
    return juce::jmax(1, juce::roundToInt(logicalHeight * scale));
}

void AnalyzerMeterControlsComponent::rebuildCachedDecor() {
    if (decorBounds.isEmpty()) {
        cachedDecorImage = {};
        return;
    }

    const auto& sourceImage = getDecorGridImage();
    const auto rasterScale = theme.metrics.assets.rasterScale;
    const auto logicalWidth = static_cast<float>(sourceImage.getWidth()) / rasterScale;
    const auto logicalHeight = static_cast<float>(sourceImage.getHeight()) / rasterScale;
    const auto widthScale = static_cast<float>(decorBounds.getWidth()) / logicalWidth;
    const auto heightScale = static_cast<float>(decorBounds.getHeight()) / logicalHeight;
    const auto scale = juce::jlimit(0.0f, 1.0f, std::min(widthScale, heightScale));
    const auto targetWidth = juce::jmax(1, juce::roundToInt(logicalWidth * scale));
    const auto targetHeight = juce::jmax(1, juce::roundToInt(logicalHeight * scale));

    decorBounds = juce::Rectangle<int>(targetWidth, targetHeight).withCentre(decorBounds.getCentre());
    cachedDecorImage = sourceImage.rescaled(targetWidth, targetHeight, juce::Graphics::highResamplingQuality);
}

const juce::Image& AnalyzerMeterControlsComponent::getDecorGridImage() {
    static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::decor_grid_png,
                                                              static_cast<size_t>(BinaryData::decor_grid_pngSize));
    return image;
}
