#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "UiTheme.h"
#include "analyzer/AnalyzerComponent.h"

//==============================================================================
class SpectrumAnalyzerAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    SpectrumAnalyzerAudioProcessorEditor(juce::AudioProcessor &processor, AnalyzerDataSource &analyzerDataSource);

    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    Ui::Theme theme;
    AnalyzerComponent analyzerComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessorEditor)
};
