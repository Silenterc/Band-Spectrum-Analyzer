#include "ui/settings/view/SettingsTimeDecaySectionComponent.h"

namespace {
    struct TimeDecayKnobDefinition {
        const char* label;
        float minimum;
        float maximum;
        float step;
        float initialValue;
        const char* suffix;
        int decimalPlaces;
    };

    constexpr std::array<TimeDecayKnobDefinition, 4> knobDefinitions{
        TimeDecayKnobDefinition{"HOLD TIME", 0.0f, 5000.0f, 10.0f, 750.0f, "ms", 0},
        TimeDecayKnobDefinition{"RMS TIME", 0.0f, 5000.0f, 10.0f, 250.0f, "ms", 0},
        TimeDecayKnobDefinition{"PEAK DECAY", 0.0f, 60.0f, 0.1f, 18.0f, "dB/s", 1},
        TimeDecayKnobDefinition{"HOLD DECAY", 0.0f, 60.0f, 0.1f, 9.0f, "dB/s", 1}
    };

    juce::String formatValueWithSuffix(const float value,
                                       const int decimalPlaces,
                                       const juce::String& suffix) {
        return juce::String(value, decimalPlaces) + " " + suffix;
    }
}

SettingsTimeDecaySectionComponent::SettingsTimeDecaySectionComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      frame(themeToUse, "TIME & DECAY") {
    addAndMakeVisible(frame);
    createKnobs();
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

    values[index] = definition.initialValue;
    knobs[index]->setConfig(std::move(config));
    knobs[index]->setValue(values[index]);
    knobs[index]->onValueChanged = [this, index](const float value) {
        values[index] = value;
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
