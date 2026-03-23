#pragma once

#include <juce_graphics/juce_graphics.h>

namespace Ui {
    enum class RasterAssetId {
        background,
        background2,
        background2Version,
        decorGrid,
        padOff,
        padOn,
        padFreezeOn,
        screen,
        screw,
        switchDown,
        switchUp,
    };

    const juce::Image &getRasterAsset(RasterAssetId assetId);

    juce::Rectangle<int> getLogicalAssetBounds(const juce::Image &image,
                                               float rasterScale,
                                               juce::Point<int> topLeft);

    void drawAssetWithin(juce::Graphics &g,
                         const juce::Image &image,
                         const juce::Rectangle<int> &destinationBounds);
}
