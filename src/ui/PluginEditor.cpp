#include "PluginEditor.h"
#include "../plugin/PluginProcessor.h"

#if JucePlugin_Build_Standalone
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

//==============================================================================
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor &audioProcessor)
    : AudioProcessorEditor(&audioProcessor),
      theme(Ui::makeTheme()),
      backgroundComponent(theme),
      mainLayout(audioProcessor, audioProcessor, audioProcessor, theme) {
    setOpaque(true);
    addAndMakeVisible(backgroundComponent);
    addAndMakeVisible(mainLayout);
    setSize(1000, 600);

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
    backgroundComponent.setBounds(getLocalBounds());
    mainLayout.setBounds(getLocalBounds());
}
