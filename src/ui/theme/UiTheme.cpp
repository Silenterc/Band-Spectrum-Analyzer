#include "UiTheme.h"

namespace Ui {
    namespace {
        float scaleFactorForPreset(const UiScalePreset scalePreset) {
            switch (scalePreset) {
                case UiScalePreset::x1:
                    return 1.0f;
                case UiScalePreset::x1_5:
                    return 1.5f;
                case UiScalePreset::x2:
                    return 2.0f;
            }

            jassertfalse;
            return 1.0f;
        }

        int scaleInt(const int value, const float factor) {
            return juce::roundToInt(static_cast<float>(value) * factor);
        }

        float scaleFloat(const float value, const float factor) {
            return value * factor;
        }
    }

    PanelMetrics PanelMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.analyzerToRackGap = scaleInt(analyzerToRackGap, factor);
        scaledMetrics.rackHeight = scaleInt(rackHeight, factor);
        scaledMetrics.analyzerToSideStripGap = scaleInt(analyzerToSideStripGap, factor);
        scaledMetrics.sideStripWidth = scaleInt(sideStripWidth, factor);
        return scaledMetrics;
    }

    MeterControlsMetrics MeterControlsMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.verticalPadding = scaleInt(verticalPadding, factor);
        scaledMetrics.horizontalPadding = scaleInt(horizontalPadding, factor);
        scaledMetrics.bottomInset = scaleInt(bottomInset, factor);
        scaledMetrics.settingsTopInset = scaleInt(settingsTopInset, factor);
        scaledMetrics.settingsRightInset = scaleInt(settingsRightInset, factor);
        scaledMetrics.settingsGap = scaleInt(settingsGap, factor);
        scaledMetrics.settingsSeparatorInset = scaleInt(settingsSeparatorInset, factor);
        scaledMetrics.settingsSeparatorThickness = scaleInt(settingsSeparatorThickness, factor);
        scaledMetrics.buttonGap = scaleInt(buttonGap, factor);
        scaledMetrics.padOpticalOffsetX = scaleInt(padOpticalOffsetX, factor);
        scaledMetrics.groupGap = scaleInt(groupGap, factor);
        scaledMetrics.padTextFontHeight = scaleFloat(padTextFontHeight, factor);
        return scaledMetrics;
    }

    AnalyzerSectionMetrics AnalyzerSectionMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.plotInset = scaleInt(plotInset, factor);
        return scaledMetrics;
    }

    AnalyzerPlotMetrics AnalyzerPlotMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.plotMarginLeft = scaleFloat(plotMarginLeft, factor);
        scaledMetrics.plotMarginRight = scaleFloat(plotMarginRight, factor);
        scaledMetrics.plotMarginTop = scaleFloat(plotMarginTop, factor);
        scaledMetrics.plotMarginBottom = scaleFloat(plotMarginBottom, factor);
        scaledMetrics.interBandGapPixels = scaleInt(interBandGapPixels, factor);
        scaledMetrics.rmsLineThickness = scaleFloat(rmsLineThickness, factor);
        scaledMetrics.rmsLineUnderlayExtraThickness = scaleFloat(rmsLineUnderlayExtraThickness, factor);
        scaledMetrics.frameExpansion = scaleFloat(frameExpansion, factor);
        scaledMetrics.frameCornerRadius = scaleFloat(frameCornerRadius, factor);
        scaledMetrics.gridLabelFontHeight = scaleFloat(gridLabelFontHeight, factor);
        scaledMetrics.gridLabelWidth = scaleInt(gridLabelWidth, factor);
        scaledMetrics.gridLabelHeight = scaleInt(gridLabelHeight, factor);
        scaledMetrics.gridLabelYOffset = scaleFloat(gridLabelYOffset, factor);
        scaledMetrics.frequencyLabelWidth = scaleInt(frequencyLabelWidth, factor);
        scaledMetrics.frequencyLabelHeight = scaleInt(frequencyLabelHeight, factor);
        scaledMetrics.frequencyLabelXHalfSpan = scaleFloat(frequencyLabelXHalfSpan, factor);
        scaledMetrics.frequencyLabelYOffset = scaleFloat(frequencyLabelYOffset, factor);
        return scaledMetrics;
    }

    TooltipMetrics TooltipMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.width = scaleFloat(width, factor);
        scaledMetrics.height = scaleFloat(height, factor);
        scaledMetrics.offsetX = scaleFloat(offsetX, factor);
        scaledMetrics.offsetY = scaleFloat(offsetY, factor);
        scaledMetrics.edgeInset = scaleFloat(edgeInset, factor);
        scaledMetrics.cornerRadius = scaleFloat(cornerRadius, factor);
        scaledMetrics.textPaddingX = scaleInt(textPaddingX, factor);
        scaledMetrics.textPaddingY = scaleInt(textPaddingY, factor);
        scaledMetrics.lineHeight = scaleInt(lineHeight, factor);
        scaledMetrics.fontHeight = scaleFloat(fontHeight, factor);
        return scaledMetrics;
    }

    RackMetrics RackMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.topInset = scaleFloat(topInset, factor);
        scaledMetrics.bottomInset = scaleFloat(bottomInset, factor);
        return scaledMetrics;
    }

    SlotMetrics SlotMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.cellPaddingX = scaleFloat(cellPaddingX, factor);
        scaledMetrics.cellPaddingY = scaleFloat(cellPaddingY, factor);
        scaledMetrics.contentOffsetY = scaleFloat(contentOffsetY, factor);
        scaledMetrics.sectionGap = scaleFloat(sectionGap, factor);
        scaledMetrics.rowGap = scaleFloat(rowGap, factor);
        scaledMetrics.textStackGap = scaleFloat(textStackGap, factor);
        scaledMetrics.sourceToggleWidth = scaleFloat(sourceToggleWidth, factor);
        scaledMetrics.modeDisplayWidth = scaleFloat(modeDisplayWidth, factor);
        scaledMetrics.modeDisplayHeight = scaleFloat(modeDisplayHeight, factor);
        scaledMetrics.swatchSize = scaleFloat(swatchSize, factor);
        scaledMetrics.actionSize = scaleFloat(actionSize, factor);
        scaledMetrics.topRowEdgeInset = scaleFloat(topRowEdgeInset, factor);
        scaledMetrics.modeActionGap = scaleFloat(modeActionGap, factor);
        scaledMetrics.actionGap = scaleFloat(actionGap, factor);
        scaledMetrics.titleFontHeight = scaleFloat(titleFontHeight, factor);
        scaledMetrics.hintFontHeight = scaleFloat(hintFontHeight, factor);
        scaledMetrics.shadowOffsetY = scaleFloat(shadowOffsetY, factor);
        scaledMetrics.cellCornerRadius = scaleFloat(cellCornerRadius, factor);
        scaledMetrics.swatchCornerRadius = scaleFloat(swatchCornerRadius, factor);
        scaledMetrics.buttonCornerRadius = scaleFloat(buttonCornerRadius, factor);
        scaledMetrics.modeHoverCornerRadius = scaleFloat(modeHoverCornerRadius, factor);
        scaledMetrics.modeTitleFontDelta = scaleFloat(modeTitleFontDelta, factor);
        scaledMetrics.sourceToggleFontDelta = scaleFloat(sourceToggleFontDelta, factor);
        scaledMetrics.sourceToggleMaxLabelHeight = scaleFloat(sourceToggleMaxLabelHeight, factor);
        scaledMetrics.sourceToggleSwitchInsetX = scaleFloat(sourceToggleSwitchInsetX, factor);
        scaledMetrics.addButtonFontHeight = scaleFloat(addButtonFontHeight, factor);
        scaledMetrics.powerIconInset = scaleFloat(powerIconInset, factor);
        scaledMetrics.cancelIconInset = scaleFloat(cancelIconInset, factor);
        scaledMetrics.snowflakeIconInset = scaleFloat(snowflakeIconInset, factor);
        scaledMetrics.screwPadding = scaleFloat(screwPadding, factor);
        scaledMetrics.opacityDragThreshold = scaleFloat(opacityDragThreshold, factor);
        return scaledMetrics;
    }

    PopupMetrics PopupMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.padding = scaleFloat(padding, factor);
        scaledMetrics.sectionGap = scaleFloat(sectionGap, factor);
        scaledMetrics.rowGap = scaleFloat(rowGap, factor);
        scaledMetrics.headerHeight = scaleFloat(headerHeight, factor);
        scaledMetrics.rowHeight = scaleFloat(rowHeight, factor);
        scaledMetrics.swatchSize = scaleFloat(swatchSize, factor);
        scaledMetrics.colourGap = scaleFloat(colourGap, factor);
        scaledMetrics.shellCornerRadius = scaleFloat(shellCornerRadius, factor);
        scaledMetrics.rowCornerRadius = scaleFloat(rowCornerRadius, factor);
        scaledMetrics.rowOutlineInset = scaleFloat(rowOutlineInset, factor);
        scaledMetrics.rowOutlineThickness = scaleFloat(rowOutlineThickness, factor);
        scaledMetrics.rowTextFontHeight = scaleFloat(rowTextFontHeight, factor);
        scaledMetrics.buttonPrimaryFillBrightness = scaleFloat(buttonPrimaryFillBrightness, factor);
        scaledMetrics.buttonSecondaryFillBrightness = scaleFloat(buttonSecondaryFillBrightness, factor);
        scaledMetrics.buttonHoverBrightness = scaleFloat(buttonHoverBrightness, factor);
        scaledMetrics.buttonPressedDarkness = scaleFloat(buttonPressedDarkness, factor);
        scaledMetrics.buttonDisabledFillAlpha = scaleFloat(buttonDisabledFillAlpha, factor);
        scaledMetrics.buttonSecondaryTextBrightness = scaleFloat(buttonSecondaryTextBrightness, factor);
        scaledMetrics.buttonDisabledTextOpacity = scaleFloat(buttonDisabledTextOpacity, factor);
        scaledMetrics.swatchInset = scaleFloat(swatchInset, factor);
        scaledMetrics.swatchHoverInset = scaleFloat(swatchHoverInset, factor);
        scaledMetrics.swatchSelectedOutlineThickness = scaleFloat(swatchSelectedOutlineThickness, factor);
        scaledMetrics.swatchOutlineThickness = scaleFloat(swatchOutlineThickness, factor);
        scaledMetrics.swatchDisabledSlashInset = scaleFloat(swatchDisabledSlashInset, factor);
        scaledMetrics.swatchDisabledSlashThickness = scaleFloat(swatchDisabledSlashThickness, factor);
        return scaledMetrics;
    }

    KnobMetrics KnobMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.width = scaleInt(width, factor);
        scaledMetrics.height = scaleInt(height, factor);
        scaledMetrics.scaleWidth = scaleInt(scaleWidth, factor);
        scaledMetrics.scaleHeight = scaleInt(scaleHeight, factor);
        scaledMetrics.scaleOffsetY = scaleInt(scaleOffsetY, factor);
        scaledMetrics.knobSide = scaleInt(knobSide, factor);
        scaledMetrics.valueWidth = scaleInt(valueWidth, factor);
        scaledMetrics.valueHeight = scaleInt(valueHeight, factor);
        scaledMetrics.valueEditorTextIndentX = scaleInt(valueEditorTextIndentX, factor);
        scaledMetrics.valueEditorTextIndentTop = scaleInt(valueEditorTextIndentTop, factor);
        scaledMetrics.labelFontHeight = scaleFloat(labelFontHeight, factor);
        scaledMetrics.valueFontHeight = scaleFloat(valueFontHeight, factor);
        scaledMetrics.labelHeight = scaleInt(labelHeight, factor);
        scaledMetrics.labelToScaleGap = scaleInt(labelToScaleGap, factor);
        scaledMetrics.scaleToValueGap = scaleInt(scaleToValueGap, factor);
        scaledMetrics.dragPixelsForFullRange = scaleFloat(dragPixelsForFullRange, factor);
        return scaledMetrics;
    }

    HorizontalSliderMetrics HorizontalSliderMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.width = scaleInt(width, factor);
        scaledMetrics.height = scaleInt(height, factor);
        scaledMetrics.sliderWidth = scaleInt(sliderWidth, factor);
        scaledMetrics.sliderHeight = scaleInt(sliderHeight, factor);
        scaledMetrics.valueWidth = scaleInt(valueWidth, factor);
        scaledMetrics.valueHeight = scaleInt(valueHeight, factor);
        scaledMetrics.valueEditorTextIndentX = scaleInt(valueEditorTextIndentX, factor);
        scaledMetrics.valueEditorTextIndentTop = scaleInt(valueEditorTextIndentTop, factor);
        scaledMetrics.labelFontHeight = scaleFloat(labelFontHeight, factor);
        scaledMetrics.valueFontHeight = scaleFloat(valueFontHeight, factor);
        scaledMetrics.labelHeight = scaleInt(labelHeight, factor);
        scaledMetrics.labelToSliderGap = scaleInt(labelToSliderGap, factor);
        scaledMetrics.sliderToValueGap = scaleInt(sliderToValueGap, factor);
        scaledMetrics.dragPixelsForFullRange = scaleFloat(dragPixelsForFullRange, factor);
        return scaledMetrics;
    }

    RectanglePadMetrics RectanglePadMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.width = scaleInt(width, factor);
        scaledMetrics.height = scaleInt(height, factor);
        scaledMetrics.iconSide = scaleInt(iconSide, factor);
        scaledMetrics.visualCenterOffsetX = scaleFloat(visualCenterOffsetX, factor);
        scaledMetrics.visualCenterOffsetY = scaleFloat(visualCenterOffsetY, factor);
        scaledMetrics.labelFontHeight = scaleFloat(labelFontHeight, factor);
        return scaledMetrics;
    }

    SettingsSectionFrameMetrics SettingsSectionFrameMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.strokeWidth = scaleFloat(strokeWidth, factor);
        scaledMetrics.cornerRadius = scaleFloat(cornerRadius, factor);
        scaledMetrics.titleFontHeight = scaleFloat(titleFontHeight, factor);
        scaledMetrics.titleHorizontalPadding = scaleInt(titleHorizontalPadding, factor);
        scaledMetrics.titleGapHeight = scaleInt(titleGapHeight, factor);
        return scaledMetrics;
    }

    SettingsPageMetrics SettingsPageMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.topInset = scaleInt(topInset, factor);
        scaledMetrics.analysisSectionHeight = scaleInt(analysisSectionHeight, factor);
        scaledMetrics.timeDecayTopGap = scaleInt(timeDecayTopGap, factor);
        scaledMetrics.timeDecaySectionWidth = scaleInt(timeDecaySectionWidth, factor);
        scaledMetrics.timeDecaySectionHeight = scaleInt(timeDecaySectionHeight, factor);
        scaledMetrics.sectionColumnGap = scaleInt(sectionColumnGap, factor);
        scaledMetrics.sectionStackGap = scaleInt(sectionStackGap, factor);
        scaledMetrics.gridSectionHeight = scaleInt(gridSectionHeight, factor);
        scaledMetrics.uiSectionHeight = scaleInt(uiSectionHeight, factor);
        scaledMetrics.frequencyRangeSectionHeight = scaleInt(frequencyRangeSectionHeight, factor);
        return scaledMetrics;
    }

    SettingsAnalysisSectionMetrics SettingsAnalysisSectionMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.buttonGap = scaleInt(buttonGap, factor);
        scaledMetrics.horizontalInset = scaleInt(horizontalInset, factor);
        return scaledMetrics;
    }

    SettingsTimeDecaySectionMetrics SettingsTimeDecaySectionMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.contentTopInset = scaleInt(contentTopInset, factor);
        scaledMetrics.horizontalInset = scaleInt(horizontalInset, factor);
        scaledMetrics.columnGap = scaleInt(columnGap, factor);
        scaledMetrics.rowGap = scaleInt(rowGap, factor);
        return scaledMetrics;
    }

    SettingsGridSectionMetrics SettingsGridSectionMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.contentTopInset = scaleInt(contentTopInset, factor);
        scaledMetrics.horizontalInset = scaleInt(horizontalInset, factor);
        scaledMetrics.columnGap = scaleInt(columnGap, factor);
        return scaledMetrics;
    }

    SettingsUiSectionMetrics SettingsUiSectionMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.horizontalInset = scaleInt(horizontalInset, factor);
        scaledMetrics.buttonGap = scaleInt(buttonGap, factor);
        return scaledMetrics;
    }

    SettingsFrequencyRangeSectionMetrics SettingsFrequencyRangeSectionMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.contentTopInset = scaleInt(contentTopInset, factor);
        scaledMetrics.horizontalInset = scaleInt(horizontalInset, factor);
        scaledMetrics.contentOffsetX = scaleInt(contentOffsetX, factor);
        scaledMetrics.toggleWidth = scaleInt(toggleWidth, factor);
        scaledMetrics.toggleLabelHeight = scaleInt(toggleLabelHeight, factor);
        scaledMetrics.toggleLabelToButtonGap = scaleInt(toggleLabelToButtonGap, factor);
        scaledMetrics.toggleToSliderGap = scaleInt(toggleToSliderGap, factor);
        scaledMetrics.sliderGap = scaleInt(sliderGap, factor);
        return scaledMetrics;
    }

    BackgroundMetrics BackgroundMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.woodSideInset = scaleInt(woodSideInset, factor);
        scaledMetrics.screwPadding = scaleInt(screwPadding, factor);
        return scaledMetrics;
    }

    SectionDividerMetrics SectionDividerMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.thickness = scaleInt(thickness, factor);
        return scaledMetrics;
    }

    AssetMetrics AssetMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.rasterScale /= factor;
        return scaledMetrics;
    }

    EditorMetrics EditorMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.initialWidth = scaleInt(initialWidth, factor);
        scaledMetrics.initialHeight = scaleInt(initialHeight, factor);
        return scaledMetrics;
    }

    PresetHeaderMetrics PresetHeaderMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.height = scaleInt(height, factor);
        scaledMetrics.topInset = scaleInt(topInset, factor);
        scaledMetrics.plotGap = scaleInt(plotGap, factor);
        scaledMetrics.logoHeight = scaleInt(logoHeight, factor);
        scaledMetrics.logoLeftInset = scaleInt(logoLeftInset, factor);
        scaledMetrics.logoOpticalYOffset = scaleInt(logoOpticalYOffset, factor);
        scaledMetrics.logoGap = scaleInt(logoGap, factor);
        scaledMetrics.displayGap = scaleInt(displayGap, factor);
        scaledMetrics.groupGap = scaleInt(groupGap, factor);
        scaledMetrics.actionGap = scaleInt(actionGap, factor);
        scaledMetrics.labelFontHeight = scaleFloat(labelFontHeight, factor);
        scaledMetrics.selectorFocusOutlineInset = scaleFloat(selectorFocusOutlineInset, factor);
        scaledMetrics.selectorFocusCornerRadius = scaleFloat(selectorFocusCornerRadius, factor);
        scaledMetrics.selectorFocusOutlineThickness = scaleFloat(selectorFocusOutlineThickness, factor);
        return scaledMetrics;
    }

    PresetPopupMetrics PresetPopupMetrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.browserWidth = scaleInt(browserWidth, factor);
        scaledMetrics.maxVisibleRows = scaleInt(maxVisibleRows, factor);
        scaledMetrics.deleteIconSize = scaleInt(deleteIconSize, factor);
        scaledMetrics.deleteIconInset = scaleInt(deleteIconInset, factor);
        scaledMetrics.savePopupWidth = scaleInt(savePopupWidth, factor);
        scaledMetrics.saveEditorHeight = scaleInt(saveEditorHeight, factor);
        scaledMetrics.saveEditorTextIndentX = scaleInt(saveEditorTextIndentX, factor);
        scaledMetrics.saveEditorTextIndentTop = scaleInt(saveEditorTextIndentTop, factor);
        scaledMetrics.saveButtonHeight = scaleInt(saveButtonHeight, factor);
        scaledMetrics.saveButtonWidth = scaleInt(saveButtonWidth, factor);
        scaledMetrics.saveButtonGap = scaleInt(saveButtonGap, factor);
        scaledMetrics.confirmPopupWidth = scaleInt(confirmPopupWidth, factor);
        scaledMetrics.confirmPopupHeight = scaleInt(confirmPopupHeight, factor);
        scaledMetrics.browserRowTextInset = scaleInt(browserRowTextInset, factor);
        scaledMetrics.titleFontHeight = scaleFloat(titleFontHeight, factor);
        scaledMetrics.saveTitleBrightness = scaleFloat(saveTitleBrightness, factor);
        scaledMetrics.confirmTitleBrightness = scaleFloat(confirmTitleBrightness, factor);
        scaledMetrics.editorOutlineAlpha = scaleFloat(editorOutlineAlpha, factor);
        scaledMetrics.editorFocusOutlineAlpha = scaleFloat(editorFocusOutlineAlpha, factor);
        scaledMetrics.currentRowOutlineAlpha = scaleFloat(currentRowOutlineAlpha, factor);
        scaledMetrics.browserRowFillBrightness = scaleFloat(browserRowFillBrightness, factor);
        scaledMetrics.factoryPresetTextBrightness = scaleFloat(factoryPresetTextBrightness, factor);
        scaledMetrics.deleteIconIdleAlpha = scaleFloat(deleteIconIdleAlpha, factor);
        scaledMetrics.titleTopGap = scaleInt(titleTopGap, factor);
        scaledMetrics.titleBottomGap = scaleInt(titleBottomGap, factor);
        scaledMetrics.statusTopGap = scaleInt(statusTopGap, factor);
        scaledMetrics.statusMinHeight = scaleInt(statusMinHeight, factor);
        scaledMetrics.statusFontHeight = scaleFloat(statusFontHeight, factor);
        return scaledMetrics;
    }

    Metrics Metrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.editor = editor.scaled(factor);
        scaledMetrics.presetHeader = presetHeader.scaled(factor);
        scaledMetrics.presetPopup = presetPopup.scaled(factor);
        scaledMetrics.panel = panel.scaled(factor);
        scaledMetrics.meterControls = meterControls.scaled(factor);
        scaledMetrics.analyzerSection = analyzerSection.scaled(factor);
        scaledMetrics.analyzerPlot = analyzerPlot.scaled(factor);
        scaledMetrics.tooltip = tooltip.scaled(factor);
        scaledMetrics.rack = rack.scaled(factor);
        scaledMetrics.slot = slot.scaled(factor);
        scaledMetrics.popup = popup.scaled(factor);
        scaledMetrics.knob = knob.scaled(factor);
        scaledMetrics.horizontalSlider = horizontalSlider.scaled(factor);
        scaledMetrics.rectanglePad = rectanglePad.scaled(factor);
        scaledMetrics.settingsSectionFrame = settingsSectionFrame.scaled(factor);
        scaledMetrics.settingsPage = settingsPage.scaled(factor);
        scaledMetrics.settingsAnalysisSection = settingsAnalysisSection.scaled(factor);
        scaledMetrics.settingsTimeDecaySection = settingsTimeDecaySection.scaled(factor);
        scaledMetrics.settingsGridSection = settingsGridSection.scaled(factor);
        scaledMetrics.settingsUiSection = settingsUiSection.scaled(factor);
        scaledMetrics.settingsFrequencyRangeSection = settingsFrequencyRangeSection.scaled(factor);
        scaledMetrics.background = background.scaled(factor);
        scaledMetrics.sectionDivider = sectionDivider.scaled(factor);
        scaledMetrics.assets = assets.scaled(factor);
        return scaledMetrics;
    }

    Theme makeTheme(const UiScalePreset scalePreset) {
        Theme theme;
        theme.editorBackground = juce::Colour::fromRGB(12, 10, 9);
        theme.analyzerBackground = juce::Colour::fromRGB(24, 20, 17);
        theme.plotBackground = juce::Colour::fromRGB(31, 26, 22);
        theme.controlSurface = juce::Colour::fromRGB(38, 32, 28);
        theme.controlSurfaceHover = juce::Colour::fromRGB(49, 41, 35);
        theme.controlBorder = juce::Colour::fromRGBA(221, 197, 167, 34);
        theme.controlText = juce::Colour::fromRGB(233, 222, 205);
        theme.subtleText = juce::Colour::fromRGBA(198, 181, 157, 138);
        theme.gridBorder = juce::Colour::fromRGBA(176, 146, 112, 42);
        theme.gridLine = juce::Colour::fromRGBA(146, 118, 88, 24);
        theme.axisText = juce::Colour::fromRGBA(232, 212, 185, 152);
        theme.tooltipBackground = juce::Colour::fromRGBA(20, 15, 12, 214);
        theme.tooltipBorder = juce::Colour::fromRGBA(154, 121, 86, 72);
        theme.tooltipText = juce::Colour::fromRGB(229, 212, 186);
        theme.hardwareMarkingLight = juce::Colour::fromRGB(231, 216, 190);
        theme.hardwareMarkingDark = juce::Colour::fromRGB(74, 52, 32);
        theme.hardwareMarkingActiveLight = juce::Colour::fromRGB(255, 220, 126);
        theme.hardwareMarkingCoolDark = juce::Colour::fromRGB(58, 72, 82);
        theme.textSelectionFill = juce::Colour::fromRGB(74, 132, 255);
        theme.textSelectionText = juce::Colour::fromRGB(250, 248, 244);
        theme.presetPopupStatusText = juce::Colour::fromRGB(214, 180, 144);
        theme.sectionDividerShadow = juce::Colour::fromRGBA(18, 15, 13, 205);
        theme.sectionDividerHighlight = juce::Colour::fromFloatRGBA(0.39f, 0.35f, 0.31f, 0.76f);
        theme.metrics = theme.metrics.scaled(scaleFactorForPreset(scalePreset));
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

    juce::Colour makeHoldIndicatorColour(const juce::Colour baseColour, const Theme &theme) {
        const auto &plotMetrics = theme.metrics.analyzerPlot;
        return baseColour.withAlpha(1.0f)
            .interpolatedWith(juce::Colours::white, plotMetrics.holdIndicatorWhiteness)
            .withAlpha(plotMetrics.holdIndicatorAlpha);
    }
}
