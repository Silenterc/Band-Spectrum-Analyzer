#pragma once

#include <juce_graphics/juce_graphics.h>

namespace Ui {
    inline juce::Rectangle<int> getLogicalAssetBounds(const juce::Image& image,
                                                      const float rasterScale,
                                                      const juce::Point<int> topLeft) {
        jassert(rasterScale > 0.0f);

        const auto width = juce::jmax(1, juce::roundToInt(static_cast<float>(image.getWidth()) / rasterScale));
        const auto height = juce::jmax(1, juce::roundToInt(static_cast<float>(image.getHeight()) / rasterScale));
        return {topLeft.x, topLeft.y, width, height};
    }

    inline void drawAssetWithin(juce::Graphics& g,
                                const juce::Image& image,
                                const juce::Rectangle<int>& destinationBounds) {
        g.drawImage(image,
                    destinationBounds.getX(),
                    destinationBounds.getY(),
                    destinationBounds.getWidth(),
                    destinationBounds.getHeight(),
                    0,
                    0,
                    image.getWidth(),
                    image.getHeight());
    }
}
