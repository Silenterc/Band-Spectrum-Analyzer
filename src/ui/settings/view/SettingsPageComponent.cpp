#include "ui/settings/view/SettingsPageComponent.h"

#include "ui/theme/UiRasterAssets.h"

SettingsPageComponent::SettingsPageComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      mockFrequencyRangeFrame(themeToUse, "FREQUENCY RANGE"),
      mockHoldTimeKnob(themeToUse),
      mockVisibleMinFrequencySlider(themeToUse),
      mockBandModeButton(themeToUse, "1/6 Oct") {
    setOpaque(true);
    addAndMakeVisible(mockFrequencyRangeFrame);
    addAndMakeVisible(mockHoldTimeKnob);
    addAndMakeVisible(mockVisibleMinFrequencySlider);
    addAndMakeVisible(mockBandModeButton);

    RasterKnobComponent::Config knobConfig;
    knobConfig.label = "HOLD TIME";
    knobConfig.minimum = 0.0f;
    knobConfig.maximum = 5000.0f;
    knobConfig.step = 10.0f;
    knobConfig.suffix = "ms";
    knobConfig.formatter = [](const float value) {
        return juce::String(juce::roundToInt(value)) + " ms";
    };
    knobConfig.parser = [](const juce::String &text) -> std::optional<float> {
        return Ui::RasterFilmstrip::parseNumericText(text, "ms");
    };
    mockHoldTimeKnob.setConfig(std::move(knobConfig));
    mockHoldTimeKnob.setValue(mockHoldTimeMs);
    mockHoldTimeKnob.onValueChanged = [this](const float value) {
        mockHoldTimeMs = value;
    };

    RasterHorizontalSliderComponent::Config sliderConfig;
    sliderConfig.label = "VISIBLE MIN FREQUENCY";
    sliderConfig.minimum = 20.0f;
    sliderConfig.maximum = 20000.0f;
    sliderConfig.step = 1.0f;
    sliderConfig.suffix = "Hz";
    sliderConfig.valueMapping = Ui::SliderValueMapping::logarithmic;
    sliderConfig.formatter = [](const float value) {
        return Ui::RasterSliderValueMapping::formatFrequency(value);
    };
    sliderConfig.parser = [](const juce::String& text) -> std::optional<float> {
        return Ui::RasterSliderValueMapping::parseFrequencyText(text);
    };
    mockVisibleMinFrequencySlider.setConfig(std::move(sliderConfig));
    mockVisibleMinFrequencySlider.setValue(mockVisibleMinFrequencyHz);
    mockVisibleMinFrequencySlider.onValueChanged = [this](const float value) {
        mockVisibleMinFrequencyHz = value;
    };

    mockBandModeButton.setActive(true);
    mockBandModeButton.onClick = [this] {
        mockBandModeButton.setActive(!mockBandModeButton.isActive());
    };
}

void SettingsPageComponent::paint(juce::Graphics& g) {
    if (cachedBackground.isValid())
        g.drawImageAt(cachedBackground, 0, 0);
}

void SettingsPageComponent::resized() {
    rebuildCachedBackground();

    const auto preferredBounds = mockHoldTimeKnob.getPreferredBounds();
    const auto contentBounds = getLocalBounds().reduced(theme.metrics.analyzerSection.plotInset);
    const auto knobX = contentBounds.getX() + juce::roundToInt(static_cast<float>(contentBounds.getWidth()) * 0.10f);
    const auto knobY = contentBounds.getY() + juce::roundToInt(static_cast<float>(contentBounds.getHeight()) * 0.20f);
    mockHoldTimeKnob.setBounds(preferredBounds.withPosition(knobX, knobY));

    const auto sliderPreferredBounds = mockVisibleMinFrequencySlider.getPreferredBounds();
    const auto sliderX = knobX + preferredBounds.getWidth() + 56;
    const auto sliderY = knobY + 4;
    mockVisibleMinFrequencySlider.setBounds(sliderPreferredBounds.withPosition(sliderX, sliderY));

    const auto buttonPreferredBounds = mockBandModeButton.getPreferredBounds();
    mockBandModeButton.setBounds(buttonPreferredBounds.withPosition(sliderX, sliderY + sliderPreferredBounds.getHeight() + 14));

    const auto framePadding = 22;
    const auto frameLeft = sliderX - framePadding;
    const auto frameTop = sliderY - 22;
    const auto frameRight = juce::jmax(sliderX + sliderPreferredBounds.getWidth(),
                                       sliderX + buttonPreferredBounds.getWidth()) + framePadding;
    const auto frameBottom = mockBandModeButton.getBottom() + framePadding;
    mockFrequencyRangeFrame.setBounds({frameLeft, frameTop, frameRight - frameLeft, frameBottom - frameTop});
}

void SettingsPageComponent::rebuildCachedBackground() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        cachedBackground = {};
        return;
    }

    cachedBackground = juce::Image(juce::Image::ARGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics graphics(cachedBackground);

    const auto& backgroundImage = Ui::getAnalyzerRasterAsset(Ui::AnalyzerRasterAssetId::background2);
    graphics.drawImage(backgroundImage,
                       bounds.getX(),
                       bounds.getY(),
                       bounds.getWidth(),
                       bounds.getHeight(),
                       0,
                       0,
                       backgroundImage.getWidth(),
                       backgroundImage.getHeight());

    Ui::drawTopCornerScrews(graphics, bounds, theme);
}
