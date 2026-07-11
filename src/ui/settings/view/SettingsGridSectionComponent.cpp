#include "ui/settings/view/SettingsGridSectionComponent.h"

#include "shared/DefaultParameterValues.h"
#include "ui/settings/model/SettingsRangeModel.h"

namespace {
    constexpr std::size_t gridMinKnobIndex = 0;
    constexpr std::size_t gridMaxKnobIndex = 1;

    struct GridKnobDefinition {
        const char* label;
        float minimum;
        float maximum;
        float step;
        bool showPositiveSign;
        void (AnalyzerSettingsActions::*dispatch)(float);
        float (*read)(const Ui::AnalyzerUiSnapshot&);
    };

    constexpr std::array<GridKnobDefinition, 3> knobDefinitions{
        GridKnobDefinition{"GRID MIN dB", Defaults::gridMinDbMin, Defaults::gridMinDbMax, Defaults::gridMinDbStep,
                           false, &AnalyzerSettingsActions::setGridMinDb,
                           [](const Ui::AnalyzerUiSnapshot& snapshot) { return snapshot.gridMinDb; }},
        GridKnobDefinition{"GRID MAX dB", Defaults::gridMaxDbMin, Defaults::gridMaxDbMax, Defaults::gridMaxDbStep,
                           true, &AnalyzerSettingsActions::setGridMaxDb,
                           [](const Ui::AnalyzerUiSnapshot& snapshot) { return snapshot.gridMaxDb; }},
        GridKnobDefinition{"GRID STEP dB", Defaults::gridStepDbMin, Defaults::gridStepDbMax, Defaults::gridStepDbStep,
                           false, &AnalyzerSettingsActions::setGridStepDb,
                           [](const Ui::AnalyzerUiSnapshot& snapshot) { return snapshot.gridStepDb; }}
    };

    juce::String formatDecibelValue(const float value, const bool showPositiveSign) {
        const auto roundedValue = juce::roundToInt(value);
        const auto prefix = showPositiveSign && roundedValue > 0 ? "+" : "";
        return juce::String(prefix) + juce::String(roundedValue) + " dB";
    }
}

SettingsGridSectionComponent::SettingsGridSectionComponent(AnalyzerSettingsActions& settingsActionsToUse,
                                                           const Ui::Theme& themeToUse)
    : settingsActions(settingsActionsToUse),
      theme(themeToUse),
      frame(themeToUse, "GRID") {
    addAndMakeVisible(frame);
    createKnobs();
}

void SettingsGridSectionComponent::applySnapshot(const Ui::AnalyzerUiSnapshot& snapshot) {
    const auto minAllowed = Ui::SettingsRangeModel::gridMinDbAllowedRange(snapshot);
    const auto maxAllowed = Ui::SettingsRangeModel::gridMaxDbAllowedRange(snapshot);
    knobs[gridMinKnobIndex]->setAllowedRange(minAllowed.minimum, minAllowed.maximum);
    knobs[gridMaxKnobIndex]->setAllowedRange(maxAllowed.minimum, maxAllowed.maximum);

    for (auto index = std::size_t{}; index < knobs.size(); ++index)
        knobs[index]->setValue(knobDefinitions[index].read(snapshot));
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

    knobs[index]->setConfig(std::move(config));
    knobs[index]->onValueChanged = [this, dispatch = definition.dispatch](const float value) {
        (settingsActions.*dispatch)(value);
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
