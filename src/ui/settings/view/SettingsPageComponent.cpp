#include "ui/settings/view/SettingsPageComponent.h"

#include "ui/theme/UiRasterAssets.h"

SettingsPageComponent::SettingsPageComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      mockHoldTimeKnob(themeToUse) {
    setOpaque(true);
    addAndMakeVisible(mockHoldTimeKnob);

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
}

void SettingsPageComponent::paint(juce::Graphics& g) {
    if (cachedBackground.isValid())
        g.drawImageAt(cachedBackground, 0, 0);
}

void SettingsPageComponent::resized() {
    rebuildCachedBackground();

    const auto preferredBounds = mockHoldTimeKnob.getPreferredBounds();
    const auto contentBounds = getLocalBounds().reduced(theme.metrics.analyzerSection.plotInset);
    const auto x = contentBounds.getX() + juce::roundToInt(static_cast<float>(contentBounds.getWidth()) * 0.10f);
    const auto y = contentBounds.getY() + juce::roundToInt(static_cast<float>(contentBounds.getHeight()) * 0.20f);
    mockHoldTimeKnob.setBounds(preferredBounds.withPosition(x, y));
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
