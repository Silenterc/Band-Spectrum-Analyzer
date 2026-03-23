#include "UiRasterAssets.h"

#include <BinaryData.h>

namespace {
    const juce::Image &getBackground() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::background_png,
                                                                  static_cast<size_t>(BinaryData::background_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getBackground2() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::background_2_png,
                                                                  static_cast<size_t>(BinaryData::background_2_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getBackground2Version() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::background_2_version_png,
                                                                  static_cast<size_t>(BinaryData::background_2_version_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getDecorGrid() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::decor_grid_png,
                                                                  static_cast<size_t>(BinaryData::decor_grid_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getPadOff() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::pad_off_png,
                                                                  static_cast<size_t>(BinaryData::pad_off_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getPadOn() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::pad_on_png,
                                                                  static_cast<size_t>(BinaryData::pad_on_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getPadFreezeOn() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::pad_freeze_on_png,
                                                                  static_cast<size_t>(BinaryData::pad_freeze_on_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getScreen() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::screen_png,
                                                                  static_cast<size_t>(BinaryData::screen_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getScrew() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::screw_png,
                                                                  static_cast<size_t>(BinaryData::screw_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getSwitchDown() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::switch_down_png,
                                                                  static_cast<size_t>(BinaryData::switch_down_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getSwitchUp() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::switch_up_png,
                                                                  static_cast<size_t>(BinaryData::switch_up_pngSize));
        jassert(image.isValid());
        return image;
    }
}

namespace Ui {
    const juce::Image &getRasterAsset(const RasterAssetId assetId) {
        switch (assetId) {
            case RasterAssetId::background:
                return getBackground();
            case RasterAssetId::background2:
                return getBackground2();
            case RasterAssetId::background2Version:
                return getBackground2Version();
            case RasterAssetId::decorGrid:
                return getDecorGrid();
            case RasterAssetId::padOff:
                return getPadOff();
            case RasterAssetId::padOn:
                return getPadOn();
            case RasterAssetId::padFreezeOn:
                return getPadFreezeOn();
            case RasterAssetId::screen:
                return getScreen();
            case RasterAssetId::screw:
                return getScrew();
            case RasterAssetId::switchDown:
                return getSwitchDown();
            case RasterAssetId::switchUp:
                return getSwitchUp();
        }

        jassertfalse;
        return getBackground();
    }

    juce::Rectangle<int> getLogicalAssetBounds(const juce::Image &image,
                                               const float rasterScale,
                                               const juce::Point<int> topLeft) {
        jassert(rasterScale > 0.0f);

        const auto width = juce::jmax(1, juce::roundToInt(static_cast<float>(image.getWidth()) / rasterScale));
        const auto height = juce::jmax(1, juce::roundToInt(static_cast<float>(image.getHeight()) / rasterScale));
        return {topLeft.x, topLeft.y, width, height};
    }

    void drawAssetWithin(juce::Graphics &g,
                         const juce::Image &image,
                         const juce::Rectangle<int> &destinationBounds) {
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
