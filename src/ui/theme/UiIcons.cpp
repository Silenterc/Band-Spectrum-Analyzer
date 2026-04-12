#include "UiIcons.h"

#include <BinaryData.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace {
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
    juce::Drawable *getIconTemplate(const IconId iconId) {
        switch (iconId) {
            case IconId::cancel: {
                static auto drawable = juce::Drawable::createFromImageData(BinaryData::cancel_svg, BinaryData::cancel_svgSize);
                return drawable.get();
            }
            case IconId::power: {
                static auto drawable = juce::Drawable::createFromImageData(BinaryData::power_svg, BinaryData::power_svgSize);
                return drawable.get();
            }
            case IconId::grip: {
                static auto drawable = juce::Drawable::createFromImageData(BinaryData::dots_svg, BinaryData::dots_svgSize);
                return drawable.get();
            }
            case IconId::settings: {
                static auto drawable = juce::Drawable::createFromImageData(BinaryData::settings_svg, BinaryData::settings_svgSize);
                return drawable.get();
            }
            case IconId::snowflake: {
                static auto drawable = juce::Drawable::createFromImageData(BinaryData::snowflake_svg, BinaryData::snowflake_svgSize);
                return drawable.get();
            }
        }

        return nullptr;
    }

    void drawIcon(juce::Graphics &g,
                  const IconId iconId,
                  const juce::Rectangle<float> &bounds,
                  const juce::Colour colour) {
        drawTemplateIcon(g, getIconTemplate(iconId), bounds, colour);
    }
}
