#include "PluginEditor.h"

//==============================================================================
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(juce::AudioProcessor &audioProcessor,
                                                                           AnalyzerDataSource &analyzerDataSource)
    : AudioProcessorEditor(&audioProcessor), analyzerComponent(analyzerDataSource) {
    addAndMakeVisible(analyzerComponent);
    setSize(920, 420);
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor() {
}

//==============================================================================
void SpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colour::fromRGB(8, 9, 11));
}

void SpectrumAnalyzerAudioProcessorEditor::resized() {
    analyzerComponent.setBounds(getLocalBounds().reduced(18));
}
