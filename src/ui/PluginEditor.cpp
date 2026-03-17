#include "PluginEditor.h"
#include "../plugin/PluginProcessor.h"

#if JucePlugin_Build_Standalone
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

//==============================================================================
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor &audioProcessor)
    : AudioProcessorEditor(&audioProcessor),
      theme(Ui::makeTheme(Ui::AccentPalette::blue)),
      analyzerPanel(audioProcessor, audioProcessor, audioProcessor, theme) {
    setOpaque(true);
    addAndMakeVisible(analyzerPanel);
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
    analyzerPanel.setBounds(getLocalBounds().reduced(theme.metrics.panel.outerPadding));
}
