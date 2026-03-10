#include "UiTheme.h"

namespace Ui {
    namespace {
        struct AccentColours {
            juce::Colour rmsBarTop;
            juce::Colour rmsBarBottom;
            juce::Colour hoveredRmsBarTop;
            juce::Colour hoveredRmsBarBottom;
            juce::Colour barTop;
            juce::Colour barBottom;
            juce::Colour hoveredBarTop;
            juce::Colour hoveredBarBottom;
        };

        AccentColours getAccentColours(const AccentPalette accentPalette) {
            switch (accentPalette) {
                case AccentPalette::blue:
                    return {
                        juce::Colour::fromRGB(100, 224, 214),
                        juce::Colour::fromRGB(42, 156, 173),
                        juce::Colour::fromRGB(164, 244, 234),
                        juce::Colour::fromRGB(86, 194, 210),
                        juce::Colour::fromRGB(101, 179, 255),
                        juce::Colour::fromRGB(39, 110, 241),
                        juce::Colour::fromRGB(163, 220, 255),
                        juce::Colour::fromRGB(77, 156, 255)
                    };
                case AccentPalette::green:
                    return {
                        juce::Colour::fromRGB(181, 228, 122),
                        juce::Colour::fromRGB(111, 176, 67),
                        juce::Colour::fromRGB(214, 244, 160),
                        juce::Colour::fromRGB(146, 204, 95),
                        juce::Colour::fromRGB(118, 212, 136),
                        juce::Colour::fromRGB(47, 156, 87),
                        juce::Colour::fromRGB(184, 236, 170),
                        juce::Colour::fromRGB(93, 204, 122)
                    };
                case AccentPalette::orange:
                    return {
                        juce::Colour::fromRGB(255, 214, 102),
                        juce::Colour::fromRGB(235, 164, 58),
                        juce::Colour::fromRGB(255, 232, 150),
                        juce::Colour::fromRGB(247, 188, 89),
                        juce::Colour::fromRGB(255, 157, 64),
                        juce::Colour::fromRGB(255, 90, 95),
                        juce::Colour::fromRGB(255, 214, 102),
                        juce::Colour::fromRGB(255, 126, 69)
                    };
                case AccentPalette::purple:
                    return {
                        juce::Colour::fromRGB(255, 163, 222),
                        juce::Colour::fromRGB(210, 104, 186),
                        juce::Colour::fromRGB(255, 205, 239),
                        juce::Colour::fromRGB(229, 139, 207),
                        juce::Colour::fromRGB(180, 131, 255),
                        juce::Colour::fromRGB(108, 82, 224),
                        juce::Colour::fromRGB(232, 189, 255),
                        juce::Colour::fromRGB(156, 122, 255)
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
        theme.gridBorder = juce::Colour::fromRGBA(255, 255, 255, 28);
        theme.gridLine = juce::Colour::fromRGBA(255, 255, 255, 20);
        theme.axisText = juce::Colour::fromRGBA(255, 255, 255, 110);
        theme.tooltipBackground = juce::Colour::fromRGBA(10, 10, 12, 220);
        theme.tooltipBorder = juce::Colour::fromRGBA(255, 255, 255, 36);
        theme.tooltipText = juce::Colours::white;
        theme.rmsBarTop = accentColours.rmsBarTop;
        theme.rmsBarBottom = accentColours.rmsBarBottom;
        theme.hoveredRmsBarTop = accentColours.hoveredRmsBarTop;
        theme.hoveredRmsBarBottom = accentColours.hoveredRmsBarBottom;
        theme.barTop = accentColours.barTop;
        theme.barBottom = accentColours.barBottom;
        theme.hoveredBarTop = accentColours.hoveredBarTop;
        theme.hoveredBarBottom = accentColours.hoveredBarBottom;
        return theme;
    }
}
