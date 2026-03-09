#include "PluginEditor.h"

#if JucePlugin_Build_Standalone
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

//==============================================================================
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(juce::AudioProcessor &audioProcessor,
                                                                           AnalyzerDataSource &analyzerDataSource)
    : AudioProcessorEditor(&audioProcessor),
      theme(Ui::makeTheme(Ui::AccentPalette::blue)),
      analyzerComponent(analyzerDataSource, theme) {
    addAndMakeVisible(analyzerComponent);
    setSize(920, 420);

    if (juce::JUCEApplicationBase::isStandaloneApp())
    {
        if (auto* pluginHolder = juce::StandalonePluginHolder::getInstance())
        {
            pluginHolder->getMuteInputValue().setValue(false);
        }
    }
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor() = default;

//==============================================================================
void SpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics &g) {
    g.fillAll(theme.editorBackground);
}

void SpectrumAnalyzerAudioProcessorEditor::resized() {
    analyzerComponent.setBounds(getLocalBounds().reduced(18));
}
