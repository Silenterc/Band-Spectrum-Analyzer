#pragma once

#include <juce_graphics/juce_graphics.h>

namespace Ui {
    struct Theme;

    enum class SharedRasterAssetId {
        background,
        buttonOff,
        buttonOn,
        padOff,
        padOn,
        padFreezeOn,
        screen,
        screw,
        textBox,
    };

    enum class AnalyzerRasterAssetId {
        background2,
        background2Version,
        decorGrid,
        switchDown,
        switchUp,
    };

    enum class ControlRasterAssetId {
        knobSmallFilmstrip,
        knobSmallScale,
    };

    const juce::Image &getSharedRasterAsset(SharedRasterAssetId assetId);
    const juce::Image &getAnalyzerRasterAsset(AnalyzerRasterAssetId assetId);
    const juce::Image &getControlRasterAsset(ControlRasterAssetId assetId);

    juce::Rectangle<int> getLogicalAssetBounds(const juce::Image &image,
                                               float rasterScale,
                                               juce::Point<int> topLeft);

    juce::Rectangle<int> getScaledAssetBoundsWithin(const juce::Image &image,
                                                    float rasterScale,
                                                    juce::Rectangle<int> availableBounds,
                                                    float scaleFactor);

    juce::Rectangle<int> getScaledInnerBounds(juce::Rectangle<int> outerBounds,
                                              float insetFraction,
                                              float scaleMultiplier);

    void drawAssetWithin(juce::Graphics &g,
                         const juce::Image &image,
                         const juce::Rectangle<int> &destinationBounds);

    void drawAssetWithin(juce::Graphics& g,
                         const juce::Image& image,
                         const juce::Rectangle<int>& destinationBounds,
                         const juce::Rectangle<int>& sourceBounds);

    void drawTopCornerScrews(juce::Graphics &g,
                             juce::Rectangle<int> bounds,
                             const Theme &theme);
}
