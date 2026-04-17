#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "EditorBackgroundComponent.h"
#include "ui/contracts/EditorPresentationStateSource.h"
#include "../theme/UiTheme.h"
#include "ui/analyzer/layout/MainLayoutComponent.h"

class SpectrumAnalyzerAudioProcessor;

//==============================================================================
class SpectrumAnalyzerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                   private EditorPresentationStateSource::Listener {
public:
    explicit SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor &processorToUse);

    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    void editorPresentationStateChanged(const Ui::EditorPresentationState &state) override;
    void applyPresentationState(const Ui::EditorPresentationState &state);

    SpectrumAnalyzerAudioProcessor &audioProcessor;
    Ui::Theme theme;
    EditorBackgroundComponent backgroundComponent;
    MainLayoutComponent mainLayout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessorEditor)
};
