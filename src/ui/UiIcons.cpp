#include "UiIcons.h"

#include <BinaryData.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace {
    juce::Drawable *getSnowflakeTemplate() {
        static auto drawable = juce::Drawable::createFromImageData(BinaryData::snowflake_svg,
                                                                   BinaryData::snowflake_svgSize);
        return drawable.get();
    }
}

namespace Ui {
    void drawSnowflakeIcon(juce::Graphics &g,
                           const juce::Rectangle<float> &bounds,
                           const juce::Colour colour) {
        auto *drawableTemplate = getSnowflakeTemplate();
        if (drawableTemplate == nullptr)
            return;

        auto drawable = drawableTemplate->createCopy();
        if (drawable == nullptr)
            return;

        // The SVG uses currentColor, which JUCE resolves to black by default.
        drawable->replaceColour(juce::Colours::black, colour);
        drawable->drawWithin(g, bounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
    }
}
