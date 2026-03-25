#include "UiIcons.h"

#include <BinaryData.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace {
    juce::Drawable *getCancelTemplate() {
        static auto drawable = juce::Drawable::createFromImageData(BinaryData::cancel_svg,
                                                                   BinaryData::cancel_svgSize);
        return drawable.get();
    }

    juce::Drawable *getPowerTemplate() {
        static auto drawable = juce::Drawable::createFromImageData(BinaryData::power_svg,
                                                                   BinaryData::power_svgSize);
        return drawable.get();
    }

    juce::Drawable *getDotsTemplate() {
        static auto drawable = juce::Drawable::createFromImageData(BinaryData::dots_svg,
                                                                   BinaryData::dots_svgSize);
        return drawable.get();
    }

    juce::Drawable *getSettingsTemplate() {
        static auto drawable = juce::Drawable::createFromImageData(BinaryData::settings_svg,
                                                                   BinaryData::settings_svgSize);
        return drawable.get();
    }

    juce::Drawable *getSnowflakeTemplate() {
        static auto drawable = juce::Drawable::createFromImageData(BinaryData::snowflake_svg,
                                                                   BinaryData::snowflake_svgSize);
        return drawable.get();
    }

    void drawTemplateIcon(juce::Graphics& g,
                          juce::Drawable* drawableTemplate,
                          const juce::Rectangle<float>& bounds,
                          const juce::Colour colour) {
        if (drawableTemplate == nullptr)
            return;

        auto drawable = drawableTemplate->createCopy();
        if (drawable == nullptr)
            return;

        drawable->replaceColour(juce::Colours::black, colour);
        drawable->drawWithin(g, bounds, juce::RectanglePlacement::centred, 1.0f);
    }
}

namespace Ui {
    void drawCancelIcon(juce::Graphics &g,
                        const juce::Rectangle<float> &bounds,
                        const juce::Colour colour) {
        drawTemplateIcon(g, getCancelTemplate(), bounds, colour);
    }

    void drawPowerIcon(juce::Graphics &g,
                       const juce::Rectangle<float> &bounds,
                       const juce::Colour colour) {
        drawTemplateIcon(g, getPowerTemplate(), bounds, colour);
    }

    void drawGripIcon(juce::Graphics &g,
                      const juce::Rectangle<float> &bounds,
                      const juce::Colour colour) {
        drawTemplateIcon(g, getDotsTemplate(), bounds, colour);
    }

    void drawSettingsIcon(juce::Graphics &g,
                          const juce::Rectangle<float> &bounds,
                          const juce::Colour colour) {
        drawTemplateIcon(g, getSettingsTemplate(), bounds, colour);
    }

    void drawSnowflakeIcon(juce::Graphics &g,
                           const juce::Rectangle<float> &bounds,
                           const juce::Colour colour) {
        drawTemplateIcon(g, getSnowflakeTemplate(), bounds, colour);
    }
}
