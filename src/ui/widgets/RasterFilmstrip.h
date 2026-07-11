#pragma once

#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

namespace Ui {
    enum class FilmstripOrientation {
        vertical,
        horizontal
    };

    struct RasterFilmstripSpec {
        int frameCount = 1;
        int frameWidth = 1;
        int frameHeight = 1;
        FilmstripOrientation orientation = FilmstripOrientation::vertical;
    };

    class RasterFilmstrip {
    public:
        static bool isValid(const juce::Image& image, const RasterFilmstripSpec& spec);
        static int frameIndexForNormalisedValue(float normalisedValue, int frameCount);
        static float normaliseValue(float value, float minimum, float maximum);
        static float snapValue(float value, float minimum, float maximum, float step);
        static juce::Rectangle<int> getFrameSourceBounds(const RasterFilmstripSpec& spec, int frameIndex);
        static void drawFrame(juce::Graphics& g,
                              const juce::Image& image,
                              const RasterFilmstripSpec& spec,
                              int frameIndex,
                              const juce::Rectangle<int>& destinationBounds);
        static std::optional<float> parseNumericText(juce::String text, juce::String suffix);
    };
}
