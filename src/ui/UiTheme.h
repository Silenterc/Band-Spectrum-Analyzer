#pragma once

#include <juce_graphics/juce_graphics.h>

#include "SignalSlotUiState.h"
#include "../shared/SignalPresetCatalog.h"

namespace Ui {
    enum class AccentPalette {
        blue,
        green,
        orange,
        purple
    };

    struct PanelMetrics {
        int analyzerToRackGap = 10;
        int rackHeight = 112;
        int analyzerToSideStripGap = 10;
        int sideStripWidth = 84;
    };

    struct MeterControlsMetrics {
        int verticalPadding = 0;
        int horizontalPadding = 0;
        int bottomInset = 2;
        int settingsTopInset = 0;
        int settingsRightInset = 0;
        int settingsGap = 26;
        int settingsSeparatorInset = 10;
        int settingsSeparatorThickness = 4;
        int buttonGap = 6;
        int padOpticalOffsetX = -1;
        int groupGap = 6;
        float padScale = 0.5f;
        float settingsPadScaleMultiplier = 1.0f;
        float settingsIconScaleMultiplier = 2.3f;
        float freezePadScaleMultiplier = 1.5f;
        float freezeIconScaleMultiplier = 1.2f;
        float decorScale = 0.9f;
        float padTextFontHeight = 16.0f;
    };

    struct AnalyzerSectionMetrics {
        int plotInset = 36;
    };

    struct RackMetrics {
        float topInset = 0.0f;
        float bottomInset = 2.0f;
    };

    struct SlotMetrics {
        float cellPaddingX = 6.0f;
        float cellPaddingY = 5.0f;
        float sectionGap = 6.0f;
        float rowGap = 6.0f;
        float textStackGap = 2.0f;
        float sourceToggleWidth = 54.0f;
        float sourceToggleHeight = 72.0f;
        float topRowHeight = 34.0f;
        float modePickerPaddingX = 12.0f;
        float swatchSize = 28.0f;
        float actionSize = 24.0f;
        float actionGap = 5.0f;
        float titleFontHeight = 15.0f;
        float hintFontHeight = 9.0f;
        float titleHeight = 17.0f;
        float hintHeight = 10.0f;
        float gripWidth = 8.0f;
        float gripHeight = 15.0f;
        float gripDotDiameter = 2.25f;
        float shadowOffsetY = 3.0f;
        float cellCornerRadius = 0.0f;
        float swatchCornerRadius = 7.0f;
        float buttonCornerRadius = 6.0f;
    };

    struct PopupMetrics {
        float padding = 8.0f;
        float sectionGap = 8.0f;
        float rowGap = 4.0f;
        float headerHeight = 14.0f;
        float rowHeight = 28.0f;
        float swatchSize = 32.0f;
        int colourColumns = 4;
        float colourGap = 4.0f;
    };

    struct BackgroundMetrics {
        int woodSideInset = 32;
        int screwPadding = 2;
    };

    struct SectionDividerMetrics {
        int thickness = 6;
    };

    struct AssetMetrics {
        float rasterScale = 2.0f;
    };

    struct Metrics {
        PanelMetrics panel;
        MeterControlsMetrics meterControls;
        AnalyzerSectionMetrics analyzerSection;
        RackMetrics rack;
        SlotMetrics slot;
        PopupMetrics popup;
        BackgroundMetrics background;
        SectionDividerMetrics sectionDivider;
        AssetMetrics assets;
    };

    struct Theme {
        juce::Colour editorBackground;
        juce::Colour analyzerBackground;
        juce::Colour plotBackground;
        juce::Colour controlSurface;
        juce::Colour controlSurfaceHover;
        juce::Colour controlBorder;
        juce::Colour controlText;
        juce::Colour subtleText;
        juce::Colour accentButton;
        juce::Colour accentButtonActive;
        juce::Colour gridBorder;
        juce::Colour gridLine;
        juce::Colour axisText;
        juce::Colour tooltipBackground;
        juce::Colour tooltipBorder;
        juce::Colour tooltipText;
        juce::Colour hardwareMarkingLight;
        juce::Colour hardwareMarkingDark;
        juce::Colour hardwareMarkingCoolDark;
        juce::Colour sectionDividerShadow;
        juce::Colour sectionDividerHighlight;
        Metrics metrics;
    };

    Theme makeTheme(AccentPalette accentPalette);
    const Shared::SignalPresetSpec &getSignalPreset(int colourIndex);
    juce::Colour getSignalPresetColour(int colourIndex);
    juce::String getSignalPresetName(int colourIndex);
}
