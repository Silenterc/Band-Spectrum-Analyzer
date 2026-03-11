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
                        juce::Colour::fromRGB(101, 179, 255),
                        juce::Colour::fromRGB(39, 110, 241)
                    };
                case AccentPalette::green:
                    return {
                        juce::Colour::fromRGB(118, 212, 136),
                        juce::Colour::fromRGB(47, 156, 87)
                    };
                case AccentPalette::orange:
                    return {
                        juce::Colour::fromRGB(255, 157, 64),
                        juce::Colour::fromRGB(255, 90, 95)
                    };
                case AccentPalette::purple:
                    return {
                        juce::Colour::fromRGB(180, 131, 255),
                        juce::Colour::fromRGB(108, 82, 224)
                    };
            }

            return getAccentColours(AccentPalette::blue);
        }
    }

    Theme makeTheme(const AccentPalette accentPalette) {
        const auto accentColours = getAccentColours(accentPalette);

        Theme theme;
        theme.editorBackground = juce::Colour::fromRGB(8, 9, 11);
        theme.analyzerBackground = juce::Colour::fromRGB(17, 18, 20);
        theme.plotBackground = juce::Colour::fromRGB(28, 31, 35);
        theme.controlSurface = juce::Colour::fromRGB(26, 28, 31);
        theme.controlSurfaceHover = juce::Colour::fromRGB(34, 37, 42);
        theme.controlBorder = juce::Colour::fromRGBA(255, 255, 255, 28);
        theme.controlText = juce::Colours::white;
        theme.subtleText = juce::Colour::fromRGBA(255, 255, 255, 120);
        theme.accentButton = accentColours.accentButton;
        theme.accentButtonActive = accentColours.accentButtonActive;
        theme.gridBorder = juce::Colour::fromRGBA(255, 255, 255, 28);
        theme.gridLine = juce::Colour::fromRGBA(255, 255, 255, 20);
        theme.axisText = juce::Colour::fromRGBA(255, 255, 255, 110);
        theme.tooltipBackground = juce::Colour::fromRGBA(10, 10, 12, 220);
        theme.tooltipBorder = juce::Colour::fromRGBA(255, 255, 255, 36);
        theme.tooltipText = juce::Colours::white;
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
