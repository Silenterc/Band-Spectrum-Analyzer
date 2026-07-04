#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "EditorBackgroundComponent.h"
#include "ui/editor/contracts/EditorContext.h"
#include "ui/editor/contracts/EditorPresentationStateSource.h"
#include "ui/theme/UiTheme.h"
#include "ui/editor/layout/MainLayoutComponent.h"

//==============================================================================
class SpectrumAnalyzerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                   private EditorPresentationStateSource::Listener {
public:
    SpectrumAnalyzerAudioProcessorEditor(juce::AudioProcessor& processorToUse,
                                         const Ui::EditorContext& context);

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
