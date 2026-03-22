#pragma once

#include <array>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../AnalyzerSettingsActions.h"
#include "../../AnalyzerUiStateSource.h"
#include "../../PadButton.h"
#include "../../UiTheme.h"

class AnalyzerMeterControlsComponent final : public juce::Component,
                                             private AnalyzerUiStateSource::Listener {
public:
    AnalyzerMeterControlsComponent(AnalyzerUiStateSource &uiStateSource,
                                   AnalyzerSettingsActions &settingsActions,
                                   const Ui::Theme &theme);
    ~AnalyzerMeterControlsComponent() override;

    void resized() override;
    void paint(juce::Graphics &g) override;

private:
    void analyzerUiStateChanged(const Ui::AnalyzerUiState &state) override;
    void syncButtonStates(const Ui::AnalyzerUiState &state);
    int getDecorPreferredHeight(int availableWidth) const;
    void rebuildCachedDecor();
    static const juce::Image& getDecorGridImage();

    AnalyzerUiStateSource &uiStateSource;
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
    Ui::AnalyzerUiState currentState;
};
