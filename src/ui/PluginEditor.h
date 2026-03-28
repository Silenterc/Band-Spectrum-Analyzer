#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "EditorBackgroundComponent.h"
#include "UiTheme.h"
#include "analyzer/view/MainLayoutComponent.h"

class SpectrumAnalyzerAudioProcessor;

//==============================================================================
class SpectrumAnalyzerAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor &processor);

    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    Ui::Theme theme;
    EditorBackgroundComponent backgroundComponent;
    MainLayoutComponent mainLayout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessorEditor)
};
