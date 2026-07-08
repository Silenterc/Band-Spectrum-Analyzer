#include "ui/settings/view/SettingsFrequencyRangeSectionComponent.h"

namespace {
    RasterHorizontalSliderComponent::Config makeFrequencySliderConfig(const juce::String& label) {
        RasterHorizontalSliderComponent::Config config;
        config.label = label;
        config.minimum = 20.0f;
        config.maximum = 20000.0f;
        config.step = 1.0f;
        config.suffix = "Hz";
        config.valueMapping = Ui::SliderValueMapping::logarithmic;
        config.formatter = [](const float value) {
            return Ui::RasterSliderValueMapping::formatFrequency(value);
        };
        config.parser = [](const juce::String& text) -> std::optional<float> {
            return Ui::RasterSliderValueMapping::parseFrequencyText(text);
        };
        return config;
    }
}

SettingsFrequencyRangeSectionComponent::SettingsFrequencyRangeSectionComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      frame(themeToUse, "FREQUENCY RANGE"),
      customRangeButton(themeToUse, {}),
      visibleMinFrequencySlider(themeToUse),
      visibleMaxFrequencySlider(themeToUse) {
    addAndMakeVisible(frame);
    addAndMakeVisible(customRangeButton);
    addAndMakeVisible(visibleMinFrequencySlider);
    addAndMakeVisible(visibleMaxFrequencySlider);

    customRangeButton.onClick = [this] {
        customRangeEnabled = !customRangeEnabled;
        updateCustomRangeButton();
    };
    customRangeButton.setIcon(Ui::IconId::power);
    configureSliders();
    updateCustomRangeButton();
}

void SettingsFrequencyRangeSectionComponent::paint(juce::Graphics& g) {
    g.setColour(theme.hardwareMarkingLight);
    g.setFont(juce::FontOptions(theme.metrics.horizontalSlider.labelFontHeight).withStyle("Bold"));
    g.drawFittedText("CUSTOM RANGE",
                     customRangeLabelBounds,
                     juce::Justification::centred,
                     1);
}

void SettingsFrequencyRangeSectionComponent::resized() {
    frame.setBounds(getLocalBounds());
    layoutControls(getLocalBounds());
}

void SettingsFrequencyRangeSectionComponent::configureSliders() {
    visibleMinFrequencySlider.setConfig(makeFrequencySliderConfig("VISIBLE MIN FREQUENCY"));
    visibleMinFrequencySlider.setValue(visibleMinFrequencyHz);
    visibleMinFrequencySlider.onValueChanged = [this](const float value) {
        visibleMinFrequencyHz = value;
    };

    visibleMaxFrequencySlider.setConfig(makeFrequencySliderConfig("VISIBLE MAX FREQUENCY"));
    visibleMaxFrequencySlider.setValue(visibleMaxFrequencyHz);
    visibleMaxFrequencySlider.onValueChanged = [this](const float value) {
        visibleMaxFrequencyHz = value;
    };
}

void SettingsFrequencyRangeSectionComponent::layoutControls(const juce::Rectangle<int> bounds) {
    const auto& metrics = theme.metrics.settingsFrequencyRangeSection;
    const auto contentBounds = bounds.reduced(metrics.horizontalInset, 0);
    const auto sliderBounds = visibleMinFrequencySlider.getPreferredBounds();
    const auto& sliderMetrics = theme.metrics.horizontalSlider;
    const auto sliderY = bounds.getY() + metrics.contentTopInset;
    const auto toggleButtonBounds = customRangeButton.getPreferredBounds();
    const auto toggleVisualLeadingInset =
        static_cast<float>(metrics.toggleWidth - toggleButtonBounds.getWidth()) / 2.0f
        + customRangeButton.getVisualCenterOffset().x;
    const auto sliderControlY = sliderY + sliderMetrics.labelHeight + sliderMetrics.labelToSliderGap;
    const auto sliderControlCentreY = sliderControlY + sliderMetrics.sliderHeight / 2;
    const auto totalControlWidth = metrics.toggleWidth
                                   + metrics.toggleToSliderGap
                                   + sliderBounds.getWidth()
                                   + metrics.sliderGap
                                   + sliderBounds.getWidth();
    const auto controlStartX = juce::roundToInt(static_cast<float>(contentBounds.getCentreX())
                                                - (static_cast<float>(totalControlWidth) + toggleVisualLeadingInset)
                                                    / 2.0f)
                               + metrics.contentOffsetX;
    const auto minSliderX = controlStartX + metrics.toggleWidth + metrics.toggleToSliderGap;
    const auto maxSliderX = minSliderX + sliderBounds.getWidth() + metrics.sliderGap;
    const auto toggleCentreX = juce::roundToInt(static_cast<float>(controlStartX + metrics.toggleWidth / 2)
                                                - customRangeButton.getVisualCenterOffset().x);
    const auto toggleCentreY = juce::roundToInt(static_cast<float>(sliderControlCentreY)
                                                - customRangeButton.getVisualCenterOffset().y);

    customRangeLabelBounds = juce::Rectangle<int>(metrics.toggleWidth, metrics.toggleLabelHeight)
                                 .withPosition(controlStartX, sliderY);
    customRangeButton.setBounds(toggleButtonBounds.withCentre({toggleCentreX, toggleCentreY}));

    visibleMinFrequencySlider.setBounds(sliderBounds.withPosition(minSliderX, sliderY));
    visibleMaxFrequencySlider.setBounds(sliderBounds.withPosition(maxSliderX, sliderY));
}

void SettingsFrequencyRangeSectionComponent::updateCustomRangeButton() {
    customRangeButton.setActive(customRangeEnabled);
}
