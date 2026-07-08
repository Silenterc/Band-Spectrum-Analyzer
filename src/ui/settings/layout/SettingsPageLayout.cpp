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

        const auto validationTop = layout.analysisSectionBounds.getBottom() + settingsMetrics.validationTopGap;
        layout.validationKnobBounds = juce::Rectangle<int>(theme.metrics.knob.width,
                                                           theme.metrics.knob.height)
                                          .withPosition(left + settingsMetrics.validationKnobLeftInset, validationTop);

        layout.validationSliderBounds = juce::Rectangle<int>(theme.metrics.horizontalSlider.width,
                                                             theme.metrics.horizontalSlider.height)
                                            .withPosition(layout.validationKnobBounds.getRight()
                                                              + settingsMetrics.validationSliderGap,
                                                          validationTop + settingsMetrics.validationSliderOffsetY);

        const auto framePadding = settingsMetrics.validationFramePadding;
        layout.validationFrequencyFrameBounds = layout.validationSliderBounds
                                                    .expanded(framePadding, framePadding)
                                                    .withBottom(layout.validationSliderBounds.getBottom() + framePadding);

        return layout;
    }
}
