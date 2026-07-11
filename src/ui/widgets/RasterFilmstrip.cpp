#include "RasterFilmstrip.h"

#include <cstdlib>
#include <cmath>

namespace Ui {
    bool RasterFilmstrip::isValid(const juce::Image& image, const RasterFilmstripSpec& spec) {
        if (!image.isValid() || spec.frameCount <= 0 || spec.frameWidth <= 0 || spec.frameHeight <= 0)
            return false;

        switch (spec.orientation) {
            case FilmstripOrientation::vertical:
                return image.getWidth() == spec.frameWidth
                       && image.getHeight() == spec.frameHeight * spec.frameCount;
            case FilmstripOrientation::horizontal:
                return image.getWidth() == spec.frameWidth * spec.frameCount
                       && image.getHeight() == spec.frameHeight;
        }

        jassertfalse;
        return false;
    }

    int RasterFilmstrip::frameIndexForNormalisedValue(const float normalisedValue, const int frameCount) {
        if (frameCount <= 1)
            return 0;

        const auto clampedValue = juce::jlimit(0.0f, 1.0f, normalisedValue);
        return juce::jlimit(0, frameCount - 1, static_cast<int>(std::floor(clampedValue * static_cast<float>(frameCount - 1))));
    }

    float RasterFilmstrip::normaliseValue(const float value, const float minimum, const float maximum) {
        if (maximum <= minimum)
            return 0.0f;

        return juce::jlimit(0.0f, 1.0f, (value - minimum) / (maximum - minimum));
    }

    float RasterFilmstrip::snapValue(const float value, const float minimum, const float maximum, const float step) {
        const auto clampedValue = juce::jlimit(minimum, maximum, value);
        if (step <= 0.0f)
            return clampedValue;

        const auto snappedSteps = std::round((clampedValue - minimum) / step);
        return juce::jlimit(minimum, maximum, minimum + static_cast<float>(snappedSteps) * step);
    }

    juce::Rectangle<int> RasterFilmstrip::getFrameSourceBounds(const RasterFilmstripSpec& spec, const int frameIndex) {
        const auto safeFrameIndex = frameIndexForNormalisedValue(
            spec.frameCount <= 1 ? 0.0f : static_cast<float>(frameIndex) / static_cast<float>(spec.frameCount - 1),
            spec.frameCount);

        switch (spec.orientation) {
            case FilmstripOrientation::vertical:
                return {0, safeFrameIndex * spec.frameHeight, spec.frameWidth, spec.frameHeight};
            case FilmstripOrientation::horizontal:
                return {safeFrameIndex * spec.frameWidth, 0, spec.frameWidth, spec.frameHeight};
        }

        jassertfalse;
        return {0, 0, spec.frameWidth, spec.frameHeight};
    }

    void RasterFilmstrip::drawFrame(juce::Graphics& g,
                                    const juce::Image& image,
                                    const RasterFilmstripSpec& spec,
                                    const int frameIndex,
                                    const juce::Rectangle<int>& destinationBounds) {
        if (!isValid(image, spec) || destinationBounds.isEmpty())
            return;

        const auto sourceBounds = getFrameSourceBounds(spec, frameIndex);
        g.drawImage(image,
                    destinationBounds.getX(),
                    destinationBounds.getY(),
                    destinationBounds.getWidth(),
                    destinationBounds.getHeight(),
                    sourceBounds.getX(),
                    sourceBounds.getY(),
                    sourceBounds.getWidth(),
                    sourceBounds.getHeight());
    }

    std::optional<float> RasterFilmstrip::parseNumericText(juce::String text, juce::String suffix) {
        text = text.trim();
        suffix = suffix.trim();

        if (suffix.isNotEmpty() && text.endsWithIgnoreCase(suffix))
            text = text.dropLastCharacters(suffix.length()).trim();

        if (text.isEmpty())
            return std::nullopt;

        const auto textWithoutPlus = text.startsWithChar('+') ? text.substring(1) : text;

        if (!textWithoutPlus.containsOnly("0123456789.-"))
            return std::nullopt;

        if (!textWithoutPlus.containsAnyOf("0123456789"))
            return std::nullopt;

        const auto utf8 = text.toRawUTF8();
        char* end = nullptr;
        const auto value = std::strtof(utf8, &end);
        if (end == utf8 || *end != '\0')
            return std::nullopt;

        return value;
    }
}
