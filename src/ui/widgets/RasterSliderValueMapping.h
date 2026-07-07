#pragma once

#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

namespace Ui {
    enum class SliderValueMapping {
        linear,
        logarithmic
    };

    class RasterSliderValueMapping {
    public:
        static float normaliseValue(float value, float minimum, float maximum, SliderValueMapping mapping);
        static float denormaliseValue(float normalisedValue, float minimum, float maximum, SliderValueMapping mapping);
        static float snapValue(float value, float minimum, float maximum, float step);
        static std::optional<float> parseFrequencyText(const juce::String& text);
        static juce::String formatFrequency(float frequencyHz);
    };
}
