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
        scaledMetrics.rowTextFontHeight = scaleFloat(rowTextFontHeight, factor);
        scaledMetrics.swatchInset = scaleFloat(swatchInset, factor);
        scaledMetrics.swatchHoverInset = scaleFloat(swatchHoverInset, factor);
        scaledMetrics.swatchSelectedOutlineThickness = scaleFloat(swatchSelectedOutlineThickness, factor);
        scaledMetrics.swatchOutlineThickness = scaleFloat(swatchOutlineThickness, factor);
        scaledMetrics.swatchDisabledSlashInset = scaleFloat(swatchDisabledSlashInset, factor);
        scaledMetrics.swatchDisabledSlashThickness = scaleFloat(swatchDisabledSlashThickness, factor);
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

    Metrics Metrics::scaled(const float factor) const {
        auto scaledMetrics = *this;
        scaledMetrics.editor = editor.scaled(factor);
        scaledMetrics.panel = panel.scaled(factor);
        scaledMetrics.meterControls = meterControls.scaled(factor);
        scaledMetrics.analyzerSection = analyzerSection.scaled(factor);
        scaledMetrics.analyzerPlot = analyzerPlot.scaled(factor);
        scaledMetrics.tooltip = tooltip.scaled(factor);
        scaledMetrics.rack = rack.scaled(factor);
        scaledMetrics.slot = slot.scaled(factor);
        scaledMetrics.popup = popup.scaled(factor);
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
        theme.hardwareMarkingCoolDark = juce::Colour::fromRGB(58, 72, 82);
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
