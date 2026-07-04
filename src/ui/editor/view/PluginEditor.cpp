#include "PluginEditor.h"

#if JucePlugin_Build_Standalone
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

//==============================================================================
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(
    juce::AudioProcessor& processorToUse,
    const Ui::EditorContext& context)
    : AudioProcessorEditor(&processorToUse),
      presentationStateSource(context.editorPresentationStateSource),
      theme(Ui::makeTheme(context.editorPresentationStateSource.getEditorPresentationState().scale)),
      backgroundComponent(theme),
      mainLayout(context.rawTraceSource,
                 context.analyzerUiSnapshotSource,
                 context.presetUiSnapshotSource,
                 context.analyzerSettingsActions,
                 context.presetActions,
                 theme) {
    setOpaque(true);
    addAndMakeVisible(backgroundComponent);
    addAndMakeVisible(mainLayout);
    setSize(theme.metrics.editor.initialWidth, theme.metrics.editor.initialHeight);
    presentationStateSource.addEditorPresentationStateListener(*this);

    if (juce::JUCEApplicationBase::isStandaloneApp())
    {
        if (auto* pluginHolder = juce::StandalonePluginHolder::getInstance())
        {
            pluginHolder->getMuteInputValue().setValue(false);
        }
    }
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor() {
    presentationStateSource.removeEditorPresentationStateListener(*this);
}

//==============================================================================
void SpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics &g) {
    g.fillAll(theme.editorBackground);
}

void SpectrumAnalyzerAudioProcessorEditor::resized() {
    backgroundComponent.setBounds(getLocalBounds());
    mainLayout.setBounds(getLocalBounds());
}

void SpectrumAnalyzerAudioProcessorEditor::editorPresentationStateChanged(const Ui::EditorPresentationState &state) {
    applyPresentationState(state);
}

void SpectrumAnalyzerAudioProcessorEditor::applyPresentationState(const Ui::EditorPresentationState &state) {
    theme = Ui::makeTheme(state.scale);
    setSize(theme.metrics.editor.initialWidth, theme.metrics.editor.initialHeight);
    repaint();
}
