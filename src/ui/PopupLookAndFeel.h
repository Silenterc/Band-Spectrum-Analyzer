#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "UiTheme.h"

class PopupLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    explicit PopupLookAndFeel(const Ui::Theme &themeToUse)
        : theme(themeToUse) {
    }

    void drawCallOutBoxBackground(juce::CallOutBox &box,
                                  juce::Graphics &g,
                                  const juce::Path &path,
                                  juce::Image &cachedImage) override {
        if (cachedImage.isNull()) {
            cachedImage = {juce::Image::ARGB, box.getWidth(), box.getHeight(), true,
                           *g.getInternalContext().getPreferredImageTypeForTemporaryImages()};
            cachedImage.setBackupEnabled(false);

            juce::Graphics shadowGraphics(cachedImage);
            juce::DropShadow(juce::Colours::black.withAlpha(0.38f), 8, {0, 2}).drawForPath(shadowGraphics, path);
        }

        g.drawImageAt(cachedImage, 0, 0);
        g.setColour(theme.controlSurface.withMultipliedBrightness(0.84f));
        g.fillPath(path);
        g.setColour(theme.sectionDividerHighlight.withMultipliedAlpha(0.28f));
        g.strokePath(path, juce::PathStrokeType(1.0f));
    }

    int getCallOutBoxBorderSize(const juce::CallOutBox &) override {
        return 8;
    }

    float getCallOutBoxCornerSize(const juce::CallOutBox &) override {
        return 4.0f;
    }

private:
    const Ui::Theme &theme;
};
