#pragma once

#include <array>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/analyzer/contracts/AnalyzerSettingsActions.h"
#include "ui/analyzer/contracts/AnalyzerUiSnapshotSource.h"
#include "ui/widgets/PadButton.h"
#include "ui/theme/UiTheme.h"

class AnalyzerMeterControlsComponent final : public juce::Component,
                                             private AnalyzerUiSnapshotSource::Listener {
public:
    AnalyzerMeterControlsComponent(AnalyzerUiSnapshotSource &uiSnapshotSource,
                                   AnalyzerSettingsActions &settingsActions,
                                   const Ui::Theme &theme);
    ~AnalyzerMeterControlsComponent() override;

    void resized() override;
    void paint(juce::Graphics &g) override;

private:
    void analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) override;
    void syncButtonStates(const Ui::AnalyzerUiSnapshot &snapshot);
    int getDecorPreferredHeight(int availableWidth) const;
    void rebuildCachedDecor();

    AnalyzerUiSnapshotSource &uiSnapshotSource;
    AnalyzerSettingsActions &settingsActions;
    const Ui::Theme &theme;
    PadButton settingsButton;
    PadButton peakButton;
    PadButton rmsButton;
    PadButton holdButton;
    PadButton freezeButton;
    juce::Rectangle<int> settingsSeparatorBounds;
    juce::Rectangle<int> decorBounds;
    juce::Image cachedDecorImage;
    Ui::AnalyzerUiSnapshot currentSnapshot;
};
