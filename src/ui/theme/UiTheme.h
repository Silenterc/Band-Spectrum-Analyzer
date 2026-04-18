#pragma once

#include <array>

#include <juce_graphics/juce_graphics.h>

#include "ui/editor/EditorPresentationState.h"
#include "../state/SignalSlotUiState.h"
#include "../../shared/SignalPresetCatalog.h"

namespace Ui {
    struct PanelMetrics {
        int analyzerToRackGap = 10;
        int rackHeight = 112;
        int analyzerToSideStripGap = 10;
        int sideStripWidth = 84;

        [[nodiscard]] PanelMetrics scaled(float factor) const;
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

        [[nodiscard]] MeterControlsMetrics scaled(float factor) const;
    };

    struct AnalyzerSectionMetrics {
        int plotInset = 36;

        [[nodiscard]] AnalyzerSectionMetrics scaled(float factor) const;
    };

    struct AnalyzerPlotMetrics {
        float plotMarginLeft = 56.0f;
        float plotMarginRight = 16.0f;
        float plotMarginTop = 16.0f;
        float plotMarginBottom = 26.0f;
        int interBandGapPixels = 1;
        float rmsLineThickness = 2.0f;
        float rmsLineWhiteness = 0.42f;
        float rmsLineAlpha = 0.98f;
        float rmsLineUnderlayAlpha = 0.82f;
        float rmsLineUnderlayExtraThickness = 1.6f;
        float frameExpansion = 0.0f;
        float frameCornerRadius = 0.0f;
        float gridLabelFontHeight = 12.0f;
        int gridLabelWidth = 48;
        int gridLabelHeight = 14;
        float gridLabelYOffset = 8.0f;
        int frequencyLabelWidth = 36;
        int frequencyLabelHeight = 16;
        float frequencyLabelXHalfSpan = 18.0f;
        float frequencyLabelYOffset = 6.0f;
        float gradientTopBrightness = 0.08f;
        float gradientBottomDarkness = 0.18f;
        float gradientMidPoint = 0.52f;
        float holdIndicatorWhiteness = 0.84f;
        float holdIndicatorAlpha = 0.96f;
        float maxUiFrequencyHz = 20000.0f;
        std::array<float, 10> frequencyScaleLabelsHz{
            20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f
        };

        [[nodiscard]] AnalyzerPlotMetrics scaled(float factor) const;
    };

    struct TooltipMetrics {
        float width = 132.0f;
        float height = 72.0f;
        float offsetX = 12.0f;
        float offsetY = 10.0f;
        float edgeInset = 8.0f;
        float cornerRadius = 8.0f;
        int textPaddingX = 10;
        int textPaddingY = 8;
        int lineHeight = 16;
        float fontHeight = 13.5f;
        size_t maxLines = 4;
        float fillTopBrightness = 0.06f;
        float fillBottomDarkness = 0.12f;
        float fillMidPoint = 0.45f;
        float highlightStartAlpha = 0.055f;
        float highlightEndAlpha = 0.0f;
        float highlightHeightFraction = 0.45f;

        [[nodiscard]] TooltipMetrics scaled(float factor) const;
    };

    struct RackMetrics {
        float topInset = 0.0f;
        float bottomInset = 2.0f;

        [[nodiscard]] RackMetrics scaled(float factor) const;
    };

    struct SlotMetrics {
        float cellPaddingX = 6.0f;
        float cellPaddingY = 4.0f;
        float contentOffsetY = 16.0f;
        float sectionGap = 6.0f;
        float rowGap = 6.0f;
        float textStackGap = 2.0f;
        float sourceToggleWidth = 44.0f;
        float modeDisplayWidth = 140.0f;
        float modeDisplayHeight = 30.0f;
        float swatchSize = 30.0f;
        float actionSize = 28.0f;
        float topRowEdgeInset = 4.0f;
        float modeActionGap = 4.0f;
        float actionGap = 8.0f;
        float actionPadScaleMultiplier = 2.4f;
        float actionPadIconScaleMultiplier = 1.0f;
        float titleFontHeight = 16.0f;
        float hintFontHeight = 10.0f;
        float shadowOffsetY = 4.0f;
        float cellCornerRadius = 0.0f;
        float swatchCornerRadius = 8.0f;
        float buttonCornerRadius = 6.0f;
        float modeHoverAlpha = 0.05f;
        float modeHoverCornerRadius = 4.0f;
        float modeTitleFontDelta = 1.0f;
        float sourceToggleFontDelta = 1.0f;
        float sourceToggleMaxLabelHeight = 12.0f;
        float sourceToggleLabelHeightFraction = 0.32f;
        float sourceToggleSwitchInsetX = 2.0f;
        float addButtonFontHeight = 36.0f;
        float powerIconInset = 4.0f;
        float cancelIconInset = 6.0f;
        float snowflakeIconInset = 4.0f;
        float screwPadding = 2.0f;
        float screwScale = 0.82f;
        float opacityDragThreshold = 4.0f;
        float opacityPixelsToValue = 0.005f;
        float draggedShadowAlpha = 0.22f;
        float borderAlphaScale = 1.35f;

        [[nodiscard]] SlotMetrics scaled(float factor) const;
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
        float shellCornerRadius = 4.0f;
        float shellBrightness = 0.85f;
        float shellBorderAlpha = 0.45f;
        float rowCornerRadius = 4.0f;
        float rowOutlineAlpha = 0.35f;
        float rowDisabledAlpha = 0.45f;
        float rowTextFontHeight = 14.0f;
        float swatchInset = 4.0f;
        float swatchHoverInset = 2.0f;
        float swatchHoverAlpha = 0.12f;
        float swatchOutlineAlpha = 0.14f;
        float swatchSelectedOutlineThickness = 2.0f;
        float swatchOutlineThickness = 1.0f;
        float swatchDisabledSlashInset = 4.0f;
        float swatchDisabledSlashThickness = 1.5f;

        [[nodiscard]] PopupMetrics scaled(float factor) const;
    };

    struct BackgroundMetrics {
        int woodSideInset = 31;
        int screwPadding = 2;

        [[nodiscard]] BackgroundMetrics scaled(float factor) const;
    };

    struct SectionDividerMetrics {
        int thickness = 6;
        float startAlpha = 0.90f;
        float endAlpha = 0.46f;
        float middleStartPosition = 0.38f;
        float middleStartAlpha = 0.78f;
        float middleEndPosition = 0.74f;
        float middleEndAlpha = 0.26f;

        [[nodiscard]] SectionDividerMetrics scaled(float factor) const;
    };

    struct AssetMetrics {
        float rasterScale = 2.0f;

        [[nodiscard]] AssetMetrics scaled(float factor) const;
    };

    struct EditorMetrics {
        int initialWidth = 1000;
        int initialHeight = 600;

        [[nodiscard]] EditorMetrics scaled(float factor) const;
    };

    struct PresetHeaderMetrics {
        int height = 40;
        int topInset = 4;
        int plotGap = 4;
        int logoHeight = 32;
        int logoLeftInset = 0;
        int logoGap = 8;
        int displayGap = 0;
        int groupGap = 8;
        int actionGap = 2;
        float buttonScale = 1.0f;
        float buttonIconScaleMultiplier = 1.0f;
        float textBoxScale = 1.17f;
        float labelFontHeight = 16.0f;

        [[nodiscard]] PresetHeaderMetrics scaled(float factor) const;
    };

    struct Metrics {
        EditorMetrics editor;
        PresetHeaderMetrics presetHeader;
        PanelMetrics panel;
        MeterControlsMetrics meterControls;
        AnalyzerSectionMetrics analyzerSection;
        AnalyzerPlotMetrics analyzerPlot;
        TooltipMetrics tooltip;
        RackMetrics rack;
        SlotMetrics slot;
        PopupMetrics popup;
        BackgroundMetrics background;
        SectionDividerMetrics sectionDivider;
        AssetMetrics assets;

        [[nodiscard]] Metrics scaled(float factor) const;
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

    Theme makeTheme(UiScalePreset scalePreset = UiScalePreset::x1);
    const Shared::SignalPresetSpec &getSignalPreset(int colourIndex);
    juce::Colour getSignalPresetColour(int colourIndex);
    juce::String getSignalPresetName(int colourIndex);
    juce::Colour makeHoldIndicatorColour(juce::Colour baseColour, const Theme &theme);
}
