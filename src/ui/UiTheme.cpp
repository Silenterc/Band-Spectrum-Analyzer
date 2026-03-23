#include "UiTheme.h"

namespace Ui {
    namespace {
        struct AccentColours {
            juce::Colour accentButton;
            juce::Colour accentButtonActive;
        };

        AccentColours getAccentColours(const AccentPalette accentPalette) {
            switch (accentPalette) {
                case AccentPalette::blue:
                    return {
                        juce::Colour::fromRGB(124, 166, 201),
                        juce::Colour::fromRGB(75, 117, 154)
                    };
                case AccentPalette::green:
                    return {
                        juce::Colour::fromRGB(136, 176, 118),
                        juce::Colour::fromRGB(83, 121, 69)
                    };
                case AccentPalette::orange:
                    return {
                        juce::Colour::fromRGB(205, 145, 92),
                        juce::Colour::fromRGB(152, 96, 58)
                    };
                case AccentPalette::purple:
                    return {
                        juce::Colour::fromRGB(157, 128, 174),
                        juce::Colour::fromRGB(102, 78, 118)
                    };
            }

            return getAccentColours(AccentPalette::blue);
        }
    }

    Theme makeTheme(const AccentPalette accentPalette) {
        const auto accentColours = getAccentColours(accentPalette);

        Theme theme;
        theme.editorBackground = juce::Colour::fromRGB(12, 10, 9);
        theme.analyzerBackground = juce::Colour::fromRGB(24, 20, 17);
        theme.plotBackground = juce::Colour::fromRGB(31, 26, 22);
        theme.controlSurface = juce::Colour::fromRGB(38, 32, 28);
        theme.controlSurfaceHover = juce::Colour::fromRGB(49, 41, 35);
        theme.controlBorder = juce::Colour::fromRGBA(221, 197, 167, 34);
        theme.controlText = juce::Colour::fromRGB(233, 222, 205);
        theme.subtleText = juce::Colour::fromRGBA(198, 181, 157, 138);
        theme.accentButton = accentColours.accentButton;
        theme.accentButtonActive = accentColours.accentButtonActive;
        theme.gridBorder = juce::Colour::fromRGBA(176, 146, 112, 42);
        theme.gridLine = juce::Colour::fromRGBA(146, 118, 88, 24);
        theme.axisText = juce::Colour::fromRGBA(222, 200, 170, 104);
        theme.tooltipBackground = juce::Colour::fromRGBA(20, 15, 12, 214);
        theme.tooltipBorder = juce::Colour::fromRGBA(154, 121, 86, 72);
        theme.tooltipText = juce::Colour::fromRGB(229, 212, 186);
        theme.hardwareMarkingLight = juce::Colour::fromRGB(231, 216, 190);
        theme.hardwareMarkingDark = juce::Colour::fromRGB(74, 52, 32);
        theme.hardwareMarkingCoolDark = juce::Colour::fromRGB(58, 72, 82);
        theme.sectionDividerShadow = juce::Colour::fromRGBA(18, 15, 13, 205);
        theme.sectionDividerHighlight = juce::Colour::fromFloatRGBA(0.39f, 0.35f, 0.31f, 0.76f);
        return theme;
    }

    const Shared::SignalPresetSpec &getSignalPreset(const int colourIndex) {
        const auto safeIndex = juce::jlimit(0, static_cast<int>(Shared::signalPresetCatalog.size()) - 1, colourIndex);
        return Shared::signalPresetCatalog[static_cast<size_t>(safeIndex)];
    }

    juce::Colour getSignalPresetColour(const int colourIndex) {
        return juce::Colour(getSignalPreset(colourIndex).argb);
    }

    juce::String getSignalPresetName(const int colourIndex) {
        return getSignalPreset(colourIndex).name;
    }
}
