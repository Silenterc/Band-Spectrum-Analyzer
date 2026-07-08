#include "ui/settings/layout/SettingsPageLayout.h"

namespace Ui {
    SettingsPageLayout SettingsPageLayoutBuilder::build(const juce::Rectangle<int> bounds,
                                                        const Theme& theme) {
        SettingsPageLayout layout;
        layout.contentBounds = bounds.reduced(theme.metrics.analyzerSection.plotInset);

        const auto& settingsMetrics = theme.metrics.settingsPage;
        const auto contentWidth = layout.contentBounds.getWidth();
        const auto left = layout.contentBounds.getX();
        const auto top = layout.contentBounds.getY() + settingsMetrics.topInset;

        layout.analysisSectionBounds = {
            left,
            top,
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

        return layout;
    }
}
