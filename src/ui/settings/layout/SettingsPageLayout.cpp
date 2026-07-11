#include "ui/settings/layout/SettingsPageLayout.h"

namespace Ui {
    namespace {
        float getSectionFrameTopBorderOffset(const SettingsSectionFrameMetrics& metrics) {
            return metrics.strokeWidth * metrics.strokeInsetMultiplier
                   + static_cast<float>(metrics.titleGapHeight) * metrics.titleTopGapMultiplier;
        }

        int getAnalyzerGraphTop(const Theme& theme) {
            return theme.metrics.presetHeader.topInset
                   + theme.metrics.presetHeader.height
                   + theme.metrics.presetHeader.plotGap;
        }
    }

    SettingsPageLayout SettingsPageLayoutBuilder::build(const juce::Rectangle<int> bounds,
                                                        const Theme& theme) {
        SettingsPageLayout layout;
        layout.contentBounds = bounds.reduced(theme.metrics.analyzerSection.plotInset);

        const auto& settingsMetrics = theme.metrics.settingsPage;
        const auto contentWidth = layout.contentBounds.getWidth();
        const auto left = layout.contentBounds.getX();
        const auto middleRowHeight = juce::jmax(settingsMetrics.timeDecaySectionHeight,
                                                settingsMetrics.gridSectionHeight
                                                    + settingsMetrics.sectionStackGap
                                                    + settingsMetrics.uiSectionHeight);
        const auto sectionTop = juce::roundToInt(static_cast<float>(getAnalyzerGraphTop(theme))
                                                 - getSectionFrameTopBorderOffset(theme.metrics.settingsSectionFrame));

        layout.analysisSectionBounds = {
            left,
            sectionTop,
            contentWidth,
            settingsMetrics.analysisSectionHeight
        };

        const auto timeDecayTop = layout.analysisSectionBounds.getBottom() + settingsMetrics.timeDecayTopGap;
        layout.timeDecaySectionBounds = {
            left,
            timeDecayTop,
            juce::jmin(settingsMetrics.timeDecaySectionWidth, contentWidth),
            settingsMetrics.timeDecaySectionHeight
        };

        const auto gridLeft = layout.timeDecaySectionBounds.getRight() + settingsMetrics.sectionColumnGap;
        const auto gridWidth = juce::jmax(0, layout.contentBounds.getRight() - gridLeft);
        layout.gridSectionBounds = {
            gridLeft,
            timeDecayTop,
            gridWidth,
            settingsMetrics.gridSectionHeight
        };

        const auto uiTop = layout.gridSectionBounds.getBottom() + settingsMetrics.sectionStackGap;
        layout.uiSectionBounds = {
            gridLeft,
            uiTop,
            gridWidth,
            settingsMetrics.uiSectionHeight
        };

        const auto frequencyRangeTop = timeDecayTop + middleRowHeight + settingsMetrics.sectionStackGap;
        layout.frequencyRangeSectionBounds = {
            left,
            frequencyRangeTop,
            contentWidth,
            settingsMetrics.frequencyRangeSectionHeight
        };

        return layout;
    }
}
