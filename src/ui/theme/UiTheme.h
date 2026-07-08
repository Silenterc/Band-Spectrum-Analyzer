#pragma once

#include <array>

#include <juce_graphics/juce_graphics.h>

#include "shared/UiScalePreset.h"
#include "ui/analyzer/rack/state/SignalSlotUiState.h"
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
        float rowOutlineInset = 0.5f;
        float rowOutlineThickness = 1.0f;
        float rowDisabledAlpha = 0.45f;
        float rowTextFontHeight = 14.0f;
        float buttonPrimaryFillBrightness = 0.18f;
        float buttonSecondaryFillBrightness = 0.92f;
        float buttonHoverBrightness = 0.08f;
        float buttonPressedDarkness = 0.12f;
        float buttonDisabledFillAlpha = 0.45f;
        float buttonSecondaryTextBrightness = 0.15f;
        float buttonDisabledTextOpacity = 0.5f;
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

    struct KnobMetrics {
        int width = 104;
        int height = 104;
        int scaleWidth = 56;
        int scaleHeight = 49;
        int scaleOffsetY = -8;
        int knobSide = 60;
        int valueWidth = 78;
        int valueHeight = 20;
        int valueEditorTextIndentX = 6;
        int valueEditorTextIndentTop = 2;
        float labelFontHeight = 13.0f;
        float valueFontHeight = 13.5f;
        int labelHeight = 18;
        int labelToScaleGap = 4;
        int scaleToValueGap = 2;
        int filmstripFrameCount = 128;
        int filmstripFrameWidth = 120;
        int filmstripFrameHeight = 120;
        float dragPixelsForFullRange = 180.0f;
        float keyboardStepMultiplier = 10.0f;

        [[nodiscard]] KnobMetrics scaled(float factor) const;
    };

    struct HorizontalSliderMetrics {
        int width = 240;
        int height = 102;
        int sliderWidth = 222;
        int sliderHeight = 52;
        int valueWidth = 112;
        int valueHeight = 20;
        int valueEditorTextIndentX = 6;
        int valueEditorTextIndentTop = 2;
        float labelFontHeight = 13.0f;
        float valueFontHeight = 13.5f;
        int labelHeight = 18;
        int labelToSliderGap = 4;
        int sliderToValueGap = 6;
        int filmstripFrameCount = 256;
        int filmstripFrameWidth = 444;
        int filmstripFrameHeight = 104;
        float dragPixelsForFullRange = 210.0f;
        float keyboardStepMultiplier = 10.0f;

        [[nodiscard]] HorizontalSliderMetrics scaled(float factor) const;
    };

    struct RectanglePadMetrics {
        int width = 104;
        int height = 44;
        float labelFontHeight = 15.0f;

        [[nodiscard]] RectanglePadMetrics scaled(float factor) const;
    };

    struct SettingsSectionFrameMetrics {
        float strokeWidth = 1.5f;
        float cornerRadius = 8.0f;
        float titleFontHeight = 17.0f;
        int titleHorizontalPadding = 12;
        int titleGapHeight = 18;
        float borderActiveBlend = 0.42f;
        float strokeInsetMultiplier = 0.5f;
        float titleTopGapMultiplier = 0.5f;
        float borderAlpha = 0.82f;
        float titleAlpha = 0.88f;

        [[nodiscard]] SettingsSectionFrameMetrics scaled(float factor) const;
    };

    struct SettingsPageMetrics {
        int topInset = 22;
        int analysisSectionHeight = 88;
        int timeDecayTopGap = 18;
        int timeDecaySectionWidth = 348;
        int timeDecaySectionHeight = 312;

        [[nodiscard]] SettingsPageMetrics scaled(float factor) const;
    };

    struct SettingsAnalysisSectionMetrics {
        int buttonGap = 10;
        int buttonGroupOffsetX = 74;
        int buttonOffsetY = 4;
        int labelWidth = 160;
        int labelRightToButtonLeftGap = 16;

        [[nodiscard]] SettingsAnalysisSectionMetrics scaled(float factor) const;
    };

    struct SettingsTimeDecaySectionMetrics {
        int contentTopInset = 42;
        int horizontalInset = 34;
        int columnGap = 36;
        int rowGap = 16;
        [[nodiscard]] SettingsTimeDecaySectionMetrics scaled(float factor) const;
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
        int logoHeight = 28;
        int logoLeftInset = 0;
        int logoOpticalYOffset = 4;
        int logoGap = 8;
        int displayGap = 0;
        int groupGap = 8;
        int actionGap = 2;
        float buttonScale = 1.0f;
        float buttonIconScaleMultiplier = 1.0f;
        float textBoxScale = 1.17f;
        float labelFontHeight = 16.0f;
        float selectorFocusOutlineAlpha = 0.9f;
        float selectorFocusOutlineInset = 0.5f;
        float selectorFocusCornerRadius = 6.0f;
        float selectorFocusOutlineThickness = 1.0f;

        [[nodiscard]] PresetHeaderMetrics scaled(float factor) const;
    };

    struct PresetPopupMetrics {
        int browserWidth = 228;
        int maxVisibleRows = 6;
        int deleteIconSize = 12;
        int deleteIconInset = 8;
        int savePopupWidth = 232;
        int saveEditorHeight = 28;
        int saveEditorTextIndentX = 6;
        int saveEditorTextIndentTop = 4;
        int saveButtonHeight = 22;
        int saveButtonWidth = 74;
        int saveButtonGap = 6;
        int confirmPopupWidth = 156;
        int confirmPopupHeight = 58;
        int browserRowTextInset = 10;
        float titleFontHeight = 15.0f;
        float saveTitleBrightness = 0.16f;
        float confirmTitleBrightness = 0.18f;
        float editorOutlineAlpha = 0.45f;
        float editorFocusOutlineAlpha = 0.65f;
        float currentRowOutlineAlpha = 0.95f;
        float browserRowFillBrightness = 0.94f;
        float factoryPresetTextBrightness = 0.12f;
        float deleteIconIdleAlpha = 0.72f;
        int titleTopGap = 6;
        int titleBottomGap = 4;
        int statusTopGap = 6;
        int statusMinHeight = 18;
        float statusFontHeight = 13.0f;

        [[nodiscard]] PresetPopupMetrics scaled(float factor) const;
    };

    struct Metrics {
        EditorMetrics editor;
        PresetHeaderMetrics presetHeader;
        PresetPopupMetrics presetPopup;
        PanelMetrics panel;
        MeterControlsMetrics meterControls;
        AnalyzerSectionMetrics analyzerSection;
        AnalyzerPlotMetrics analyzerPlot;
        TooltipMetrics tooltip;
        RackMetrics rack;
        SlotMetrics slot;
        PopupMetrics popup;
        KnobMetrics knob;
        HorizontalSliderMetrics horizontalSlider;
        RectanglePadMetrics rectanglePad;
        SettingsSectionFrameMetrics settingsSectionFrame;
        SettingsPageMetrics settingsPage;
        SettingsAnalysisSectionMetrics settingsAnalysisSection;
        SettingsTimeDecaySectionMetrics settingsTimeDecaySection;
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
        juce::Colour hardwareMarkingActiveLight;
        juce::Colour hardwareMarkingCoolDark;
        juce::Colour textSelectionFill;
        juce::Colour textSelectionText;
        juce::Colour presetPopupStatusText;
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
