#include "ui/settings/view/SettingsFrequencyRangeSectionComponent.h"

#include "shared/DefaultParameterValues.h"
#include "ui/settings/model/SettingsRangeModel.h"

namespace {
    RasterHorizontalSliderComponent::Config makeFrequencySliderConfig(const juce::String& label,
                                                                      const float minimum,
                                                                      const float maximum,
                                                                      const float step) {
        RasterHorizontalSliderComponent::Config config;
        config.label = label;
        config.minimum = minimum;
        config.maximum = maximum;
        config.step = step;
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

SettingsFrequencyRangeSectionComponent::SettingsFrequencyRangeSectionComponent(
    AnalyzerSettingsActions& settingsActionsToUse,
    const Ui::Theme& themeToUse)
    : settingsActions(settingsActionsToUse),
      theme(themeToUse),
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
        settingsActions.setCustomFrequencyRangeEnabled(customRangeEnabled);
    };
    customRangeButton.setIcon(Ui::IconId::power);
    configureSliders();
    updateCustomRangeButton();
}

void SettingsFrequencyRangeSectionComponent::applySnapshot(const Ui::AnalyzerUiSnapshot& snapshot) {
    customRangeEnabled = snapshot.useCustomFrequencyRange;
    updateCustomRangeButton();

    const auto minAllowed = Ui::SettingsRangeModel::visibleMinFrequencyAllowedRange(snapshot);
    const auto maxAllowed = Ui::SettingsRangeModel::visibleMaxFrequencyAllowedRange(snapshot);
    visibleMinFrequencySlider.setAllowedRange(minAllowed.minimum, minAllowed.maximum);
    visibleMaxFrequencySlider.setAllowedRange(maxAllowed.minimum, maxAllowed.maximum);
    visibleMinFrequencySlider.setValue(snapshot.visibleMinFrequencyHz);
    visibleMaxFrequencySlider.setValue(snapshot.visibleMaxFrequencyHz);
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
    visibleMinFrequencySlider.setConfig(makeFrequencySliderConfig("VISIBLE MIN FREQUENCY",
                                                                  Defaults::visibleMinFrequencyHzMin,
                                                                  Defaults::visibleMinFrequencyHzMax,
                                                                  Defaults::visibleMinFrequencyHzStep));
    visibleMinFrequencySlider.onValueChanged = [this](const float value) {
        settingsActions.setVisibleMinFrequencyHz(value);
    };

    visibleMaxFrequencySlider.setConfig(makeFrequencySliderConfig("VISIBLE MAX FREQUENCY",
                                                                  Defaults::visibleMaxFrequencyHzMin,
                                                                  Defaults::visibleMaxFrequencyHzMax,
                                                                  Defaults::visibleMaxFrequencyHzStep));
    visibleMaxFrequencySlider.onValueChanged = [this](const float value) {
        settingsActions.setVisibleMaxFrequencyHz(value);
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
