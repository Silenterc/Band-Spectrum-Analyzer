#include "RasterSliderValueMapping.h"

#include <cmath>

#include "RasterFilmstrip.h"

namespace Ui {
    namespace {
        bool canUseLogarithmicMapping(const float minimum, const float maximum) {
            return minimum > 0.0f && maximum > minimum;
        }
    }

    float RasterSliderValueMapping::normaliseValue(const float value,
                                                   const float minimum,
                                                   const float maximum,
                                                   const SliderValueMapping mapping) {
        if (mapping != SliderValueMapping::logarithmic)
            return RasterFilmstrip::normaliseValue(value, minimum, maximum);

        if (!canUseLogarithmicMapping(minimum, maximum)) {
            jassertfalse;
            return RasterFilmstrip::normaliseValue(value, minimum, maximum);
        }

        const auto clampedValue = juce::jlimit(minimum, maximum, value);
        return juce::jlimit(0.0f,
                            1.0f,
                            std::log(clampedValue / minimum) / std::log(maximum / minimum));
    }

    float RasterSliderValueMapping::denormaliseValue(const float normalisedValue,
                                                     const float minimum,
                                                     const float maximum,
                                                     const SliderValueMapping mapping) {
        const auto clampedNormalised = juce::jlimit(0.0f, 1.0f, normalisedValue);
        if (mapping != SliderValueMapping::logarithmic)
            return minimum + (maximum - minimum) * clampedNormalised;

        if (!canUseLogarithmicMapping(minimum, maximum)) {
            jassertfalse;
            return minimum + (maximum - minimum) * clampedNormalised;
        }

        return minimum * std::pow(maximum / minimum, clampedNormalised);
    }

    float RasterSliderValueMapping::snapValue(const float value,
                                              const float minimum,
                                              const float maximum,
                                              const float step) {
        return RasterFilmstrip::snapValue(value, minimum, maximum, step);
    }

    std::optional<float> RasterSliderValueMapping::parseFrequencyText(const juce::String& text) {
        auto trimmedText = text.trim();
        if (trimmedText.isEmpty())
            return std::nullopt;

        auto multiplier = 1.0f;
        if (trimmedText.endsWithIgnoreCase("kHz")) {
            multiplier = 1000.0f;
            trimmedText = trimmedText.dropLastCharacters(3).trim();
        } else if (trimmedText.endsWithIgnoreCase("Hz")) {
            trimmedText = trimmedText.dropLastCharacters(2).trim();
        }

        const auto parsedValue = RasterFilmstrip::parseNumericText(trimmedText, {});
        if (!parsedValue.has_value())
            return std::nullopt;

        return *parsedValue * multiplier;
    }

    juce::String RasterSliderValueMapping::formatFrequency(const float frequencyHz) {
        if (frequencyHz >= 1000.0f)
            return juce::String(frequencyHz / 1000.0f, 1) + " kHz";

        return juce::String(juce::roundToInt(frequencyHz)) + " Hz";
    }
}
