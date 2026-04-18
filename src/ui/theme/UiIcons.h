#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Ui {
    enum class IconId {
        cancel,
        headphones,
        left,
        power,
        reset,
        right,
        save,
        settings,
        snowflake,
    };

    juce::Drawable *getIconTemplate(IconId iconId);

    void drawIcon(juce::Graphics &g,
                  IconId iconId,
                  const juce::Rectangle<float> &bounds,
                  juce::Colour colour);
}
