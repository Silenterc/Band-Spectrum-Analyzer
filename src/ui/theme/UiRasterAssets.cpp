#include "UiRasterAssets.h"

#include "UiTheme.h"

#include <BinaryData.h>

namespace {
    const juce::Image &getBackground() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::background_png,
                                                                  static_cast<size_t>(BinaryData::background_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getButtonOff() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::button_off_png,
                                                                  static_cast<size_t>(BinaryData::button_off_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getButtonOn() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::button_on_png,
                                                                  static_cast<size_t>(BinaryData::button_on_pngSize));
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

    const juce::Image &getTextBox() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::text_box_png,
                                                                  static_cast<size_t>(BinaryData::text_box_pngSize));
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

    const juce::Image &getKnobSmallFilmstrip() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::Knob_small_png,
                                                                  static_cast<size_t>(BinaryData::Knob_small_pngSize));
        jassert(image.isValid());
        return image;
    }

    const juce::Image &getKnobSmallScale() {
        static const auto image = juce::ImageFileFormat::loadFrom(BinaryData::scale_small_knob_png,
                                                                  static_cast<size_t>(BinaryData::scale_small_knob_pngSize));
        jassert(image.isValid());
        return image;
    }
}

namespace Ui {
    const juce::Image &getSharedRasterAsset(const SharedRasterAssetId assetId) {
        switch (assetId) {
            case SharedRasterAssetId::background:
                return getBackground();
            case SharedRasterAssetId::buttonOff:
                return getButtonOff();
            case SharedRasterAssetId::buttonOn:
                return getButtonOn();
            case SharedRasterAssetId::padOff:
                return getPadOff();
            case SharedRasterAssetId::padOn:
                return getPadOn();
            case SharedRasterAssetId::padFreezeOn:
                return getPadFreezeOn();
            case SharedRasterAssetId::screen:
                return getScreen();
            case SharedRasterAssetId::screw:
                return getScrew();
            case SharedRasterAssetId::textBox:
                return getTextBox();
        }

        jassertfalse;
        return getBackground();
    }

    const juce::Image &getAnalyzerRasterAsset(const AnalyzerRasterAssetId assetId) {
        switch (assetId) {
            case AnalyzerRasterAssetId::background2:
                return getBackground2();
            case AnalyzerRasterAssetId::background2Version:
                return getBackground2Version();
            case AnalyzerRasterAssetId::decorGrid:
                return getDecorGrid();
            case AnalyzerRasterAssetId::switchDown:
                return getSwitchDown();
            case AnalyzerRasterAssetId::switchUp:
                return getSwitchUp();
        }

        jassertfalse;
        return getBackground2();
    }

    const juce::Image &getControlRasterAsset(const ControlRasterAssetId assetId) {
        switch (assetId) {
            case ControlRasterAssetId::knobSmallFilmstrip:
                return getKnobSmallFilmstrip();
            case ControlRasterAssetId::knobSmallScale:
                return getKnobSmallScale();
        }

        jassertfalse;
        return getKnobSmallFilmstrip();
    }

    juce::Rectangle<int> getLogicalAssetBounds(const juce::Image &image,
                                               const float rasterScale,
                                               const juce::Point<int> topLeft) {
        jassert(rasterScale > 0.0f);

        const auto width = juce::jmax(1, juce::roundToInt(static_cast<float>(image.getWidth()) / rasterScale));
        const auto height = juce::jmax(1, juce::roundToInt(static_cast<float>(image.getHeight()) / rasterScale));
        return {topLeft.x, topLeft.y, width, height};
    }

    juce::Rectangle<int> getScaledAssetBoundsWithin(const juce::Image &image,
                                                    const float rasterScale,
                                                    const juce::Rectangle<int> availableBounds,
                                                    const float scaleFactor) {
        jassert(rasterScale > 0.0f);

        const auto logicalWidth = static_cast<float>(image.getWidth()) / rasterScale;
        const auto logicalHeight = static_cast<float>(image.getHeight()) / rasterScale;
        const auto widthScale = static_cast<float>(juce::jmax(1, availableBounds.getWidth())) / logicalWidth;
        const auto fitScale = juce::jlimit(0.0f, 1.0f, widthScale) * juce::jmax(0.0f, scaleFactor);
        const auto targetWidth = juce::jmax(1, juce::roundToInt(logicalWidth * fitScale));
        const auto targetHeight = juce::jmax(1, juce::roundToInt(logicalHeight * fitScale));
        return juce::Rectangle<int>(targetWidth, targetHeight).withCentre(availableBounds.getCentre());
    }

    juce::Rectangle<int> getScaledInnerBounds(const juce::Rectangle<int> outerBounds,
                                              const float insetFraction,
                                              const float scaleMultiplier) {
        auto innerBounds = outerBounds.toFloat().reduced(static_cast<float>(outerBounds.getWidth()) * insetFraction);
        innerBounds = innerBounds.withSizeKeepingCentre(innerBounds.getWidth() * juce::jmax(0.0f, scaleMultiplier),
                                                        innerBounds.getHeight() * juce::jmax(0.0f, scaleMultiplier));
        return innerBounds.getSmallestIntegerContainer();
    }

    void drawAssetWithin(juce::Graphics &g,
                         const juce::Image &image,
                         const juce::Rectangle<int> &destinationBounds) {
        drawAssetWithin(g,
                        image,
                        destinationBounds,
                        {0, 0, image.getWidth(), image.getHeight()});
    }

    void drawAssetWithin(juce::Graphics& g,
                         const juce::Image& image,
                         const juce::Rectangle<int>& destinationBounds,
                         const juce::Rectangle<int>& sourceBounds) {
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

    void drawTopCornerScrews(juce::Graphics &g,
                             const juce::Rectangle<int> bounds,
                             const Theme &theme) {
        const auto& screw = getSharedRasterAsset(SharedRasterAssetId::screw);
        const auto rasterScale = theme.metrics.assets.rasterScale;
        const auto screwPadding = theme.metrics.background.screwPadding;
        const auto topLeftBounds = getLogicalAssetBounds(screw, rasterScale, {screwPadding, screwPadding});
        const auto topRightBounds = getLogicalAssetBounds(
            screw,
            rasterScale,
            {bounds.getWidth() - screwPadding - topLeftBounds.getWidth(), screwPadding});

        drawAssetWithin(g, screw, topLeftBounds);
        drawAssetWithin(g, screw, topRightBounds);
    }
}
