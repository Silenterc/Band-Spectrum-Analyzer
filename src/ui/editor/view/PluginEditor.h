#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "EditorBackgroundComponent.h"
#include "display/analyzer/contracts/AnalyzerRawTraceSource.h"
#include "ui/analyzer/contracts/AnalyzerSettingsActions.h"
#include "ui/analyzer/contracts/AnalyzerUiSnapshotSource.h"
#include "ui/editor/contracts/EditorPresentationStateSource.h"
#include "ui/presets/contracts/PresetActions.h"
#include "ui/presets/contracts/PresetUiSnapshotSource.h"
#include "ui/theme/UiTheme.h"
#include "ui/editor/layout/MainLayoutComponent.h"

//==============================================================================
class SpectrumAnalyzerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                   private EditorPresentationStateSource::Listener {
public:
    SpectrumAnalyzerAudioProcessorEditor(juce::AudioProcessor& processorToUse,
                                         AnalyzerRawTraceSource& rawTraceSource,
                                         AnalyzerUiSnapshotSource& analyzerUiSnapshotSource,
                                         PresetUiSnapshotSource& presetUiSnapshotSource,
                                         AnalyzerSettingsActions& analyzerSettingsActions,
                                         PresetActions& presetActions,
                                         EditorPresentationStateSource& editorPresentationStateSource);

    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    void editorPresentationStateChanged(const Ui::EditorPresentationState &state) override;
    void applyPresentationState(const Ui::EditorPresentationState &state);

    EditorPresentationStateSource& presentationStateSource;
    Ui::Theme theme;
    EditorBackgroundComponent backgroundComponent;
    MainLayoutComponent mainLayout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessorEditor)
};
