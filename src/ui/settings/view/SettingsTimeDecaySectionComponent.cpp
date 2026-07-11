#include "ui/settings/view/SettingsTimeDecaySectionComponent.h"

#include "shared/DefaultParameterValues.h"

namespace {
    struct TimeDecayKnobDefinition {
        const char* label;
        float minimum;
        float maximum;
        float step;
        const char* suffix;
        int decimalPlaces;
        void (AnalyzerSettingsActions::*dispatch)(float);
        float (*read)(const Analyzer::MeterSettings&);
    };

    constexpr std::array<TimeDecayKnobDefinition, 4> knobDefinitions{
        TimeDecayKnobDefinition{"HOLD TIME", Defaults::holdMsMin, Defaults::holdMsMax, Defaults::holdMsStep,
                                "ms", 0, &AnalyzerSettingsActions::setHoldTimeMs,
                                [](const Analyzer::MeterSettings& settings) { return settings.holdMs; }},
        TimeDecayKnobDefinition{"RMS TIME", Defaults::rmsWindowMsMin, Defaults::rmsWindowMsMax, Defaults::rmsWindowMsStep,
                                "ms", 0, &AnalyzerSettingsActions::setRmsWindowMs,
                                [](const Analyzer::MeterSettings& settings) { return settings.rmsWindowMs; }},
        TimeDecayKnobDefinition{"PEAK DECAY", Defaults::peakDecayDbPerSecondMin, Defaults::peakDecayDbPerSecondMax,
                                Defaults::peakDecayDbPerSecondStep,
                                "dB/s", 1, &AnalyzerSettingsActions::setPeakDecayDbPerSecond,
                                [](const Analyzer::MeterSettings& settings) { return settings.peakDecayDbPerSecond; }},
        TimeDecayKnobDefinition{"HOLD DECAY", Defaults::holdDecayDbPerSecondMin, Defaults::holdDecayDbPerSecondMax,
                                Defaults::holdDecayDbPerSecondStep,
                                "dB/s", 1, &AnalyzerSettingsActions::setHoldDecayDbPerSecond,
                                [](const Analyzer::MeterSettings& settings) { return settings.holdDecayDbPerSecond; }}
    };

    juce::String formatValueWithSuffix(const float value,
                                       const int decimalPlaces,
                                       const juce::String& suffix) {
        return juce::String(value, decimalPlaces) + " " + suffix;
    }
}

SettingsTimeDecaySectionComponent::SettingsTimeDecaySectionComponent(AnalyzerSettingsActions& settingsActionsToUse,
                                                                     const Ui::Theme& themeToUse)
    : settingsActions(settingsActionsToUse),
      theme(themeToUse),
      frame(themeToUse, "TIME & DECAY") {
    addAndMakeVisible(frame);
    createKnobs();
}

void SettingsTimeDecaySectionComponent::applySnapshot(const Ui::AnalyzerUiSnapshot& snapshot) {
    for (auto index = std::size_t{}; index < knobs.size(); ++index)
        knobs[index]->setValue(knobDefinitions[index].read(snapshot.meterSettings));
}

void SettingsTimeDecaySectionComponent::resized() {
    frame.setBounds(getLocalBounds());
    layoutKnobs(getLocalBounds());
}

void SettingsTimeDecaySectionComponent::createKnobs() {
    for (auto index = std::size_t{}; index < knobs.size(); ++index) {
        knobs[index] = std::make_unique<RasterKnobComponent>(theme);
        configureKnob(index);
        addAndMakeVisible(*knobs[index]);
    }
}

void SettingsTimeDecaySectionComponent::configureKnob(const std::size_t index) {
    const auto& definition = knobDefinitions[index];

    RasterKnobComponent::Config config;
    config.label = definition.label;
    config.minimum = definition.minimum;
    config.maximum = definition.maximum;
    config.step = definition.step;
    config.suffix = definition.suffix;
    config.formatter = [decimalPlaces = definition.decimalPlaces,
                        suffix = juce::String(definition.suffix)](const float value) {
        return formatValueWithSuffix(value, decimalPlaces, suffix);
    };
    config.parser = [suffix = juce::String(definition.suffix)](const juce::String& text) -> std::optional<float> {
        return Ui::RasterFilmstrip::parseNumericText(text, suffix);
    };

    knobs[index]->setConfig(std::move(config));
    knobs[index]->onValueChanged = [this, dispatch = definition.dispatch](const float value) {
        (settingsActions.*dispatch)(value);
    };
}

void SettingsTimeDecaySectionComponent::layoutKnobs(const juce::Rectangle<int> bounds) {
    const auto& metrics = theme.metrics.settingsTimeDecaySection;
    const auto knobBounds = juce::Rectangle<int>(theme.metrics.knob.width, theme.metrics.knob.height);
    const auto totalWidth = knobBounds.getWidth() * 2 + metrics.columnGap;
    const auto startX = bounds.getCentreX() - totalWidth / 2;
    const auto startY = bounds.getY() + metrics.contentTopInset;

    for (auto index = std::size_t{}; index < knobs.size(); ++index) {
        const auto column = static_cast<int>(index % 2);
        const auto row = static_cast<int>(index / 2);
        const auto x = startX + column * (knobBounds.getWidth() + metrics.columnGap);
        const auto y = startY + row * (knobBounds.getHeight() + metrics.rowGap);

        knobs[index]->setBounds(knobBounds.withPosition(x, y));
    }
}
