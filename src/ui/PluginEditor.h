#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "UiTheme.h"
#include "analyzer/AnalyzerPanelComponent.h"

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
    AnalyzerPanelComponent analyzerPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessorEditor)
};
