#pragma once

#include <juce_graphics/juce_graphics.h>

namespace Ui {
    void drawCancelIcon(juce::Graphics &g,
                        const juce::Rectangle<float> &bounds,
                        juce::Colour colour);

    void drawPowerIcon(juce::Graphics &g,
                       const juce::Rectangle<float> &bounds,
                       juce::Colour colour);

    void drawGripIcon(juce::Graphics &g,
                      const juce::Rectangle<float> &bounds,
                      juce::Colour colour);

    void drawSettingsIcon(juce::Graphics &g,
                          const juce::Rectangle<float> &bounds,
                          juce::Colour colour);

    void drawSnowflakeIcon(juce::Graphics &g,
                           const juce::Rectangle<float> &bounds,
                           juce::Colour colour);
}
