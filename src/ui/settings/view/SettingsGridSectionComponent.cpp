#include "ui/settings/view/SettingsGridSectionComponent.h"

namespace {
    struct GridKnobDefinition {
        const char* label;
        float minimum;
        float maximum;
        float step;
        float initialValue;
        bool showPositiveSign;
    };

    constexpr std::array<GridKnobDefinition, 3> knobDefinitions{
        GridKnobDefinition{"GRID MIN dB", -120.0f, 0.0f, 1.0f, -90.0f, false},
        GridKnobDefinition{"GRID MAX dB", -60.0f, 24.0f, 1.0f, 6.0f, true},
        GridKnobDefinition{"GRID STEP dB", 1.0f, 24.0f, 1.0f, 6.0f, false}
    };

    juce::String formatDecibelValue(const float value, const bool showPositiveSign) {
        const auto roundedValue = juce::roundToInt(value);
        const auto prefix = showPositiveSign && roundedValue > 0 ? "+" : "";
        return juce::String(prefix) + juce::String(roundedValue) + " dB";
    }
}

SettingsGridSectionComponent::SettingsGridSectionComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      frame(themeToUse, "GRID") {
    addAndMakeVisible(frame);
    createKnobs();
}

void SettingsGridSectionComponent::resized() {
    frame.setBounds(getLocalBounds());
    layoutKnobs(getLocalBounds());
}

void SettingsGridSectionComponent::createKnobs() {
    for (auto index = std::size_t{}; index < knobs.size(); ++index) {
        knobs[index] = std::make_unique<RasterKnobComponent>(theme);
        configureKnob(index);
        addAndMakeVisible(*knobs[index]);
    }
}

void SettingsGridSectionComponent::configureKnob(const std::size_t index) {
    const auto& definition = knobDefinitions[index];

    RasterKnobComponent::Config config;
    config.label = definition.label;
    config.minimum = definition.minimum;
    config.maximum = definition.maximum;
    config.step = definition.step;
    config.suffix = "dB";
    config.formatter = [showPositiveSign = definition.showPositiveSign](const float value) {
        return formatDecibelValue(value, showPositiveSign);
    };
    config.parser = [](const juce::String& text) -> std::optional<float> {
        return Ui::RasterFilmstrip::parseNumericText(text, "dB");
    };

    values[index] = definition.initialValue;
    knobs[index]->setConfig(std::move(config));
    knobs[index]->setValue(values[index]);
    knobs[index]->onValueChanged = [this, index](const float value) {
        values[index] = value;
    };
}

void SettingsGridSectionComponent::layoutKnobs(const juce::Rectangle<int> bounds) {
    const auto& metrics = theme.metrics.settingsGridSection;
    const auto contentBounds = bounds.reduced(metrics.horizontalInset, 0);
    const auto knobBounds = juce::Rectangle<int>(theme.metrics.knob.width, theme.metrics.knob.height);
    const auto totalWidth = knobBounds.getWidth() * static_cast<int>(knobs.size())
                            + metrics.columnGap * (static_cast<int>(knobs.size()) - 1);
    const auto startX = contentBounds.getCentreX() - totalWidth / 2;
    const auto y = bounds.getY() + metrics.contentTopInset;

    for (auto index = std::size_t{}; index < knobs.size(); ++index) {
        const auto column = static_cast<int>(index);
        const auto x = startX + column * (knobBounds.getWidth() + metrics.columnGap);
        knobs[index]->setBounds(knobBounds.withPosition(x, y));
    }
}
