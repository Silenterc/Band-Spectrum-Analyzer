#pragma once

#include <optional>

#include <juce_audio_processors/juce_audio_processors.h>

#include "ParamIDs.h"
#include "ParamSpec.h"

//==============================================================================
class SpectrumAnalyzerAudioProcessor final : public juce::AudioProcessor {
public:
    //==============================================================================
    SpectrumAnalyzerAudioProcessor();

    ~SpectrumAnalyzerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;

    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    //=======PRESETS================================================================
    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(int index, const juce::String &newName) override;

    //=======SAVING & LOADING STATE/PRESETS=========================================
    void getStateInformation(juce::MemoryBlock &destData) override;

    void setStateInformation(const void *data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState &getParameters() {
        return parameters;
    }

    const juce::AudioProcessorValueTreeState &getParameters() const {
        return parameters;
    }

private:
    struct ParameterState {
        ParamSpec::AnalysisMode analysisMode;
        ParamSpec::BandMode bandMode;
        bool showRms;
        bool showPeak;
        bool showHold;
        float holdMs;
        float gridMinDb;
        float gridMaxDb;
        float gridStepDb;
    };

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float> *analysisModeParam = nullptr;
    std::atomic<float> *bandModeParam = nullptr;

    std::atomic<float> *showRmsParam = nullptr;
    std::atomic<float> *showPeakParam = nullptr;
    std::atomic<float> *showHoldParam = nullptr;

    std::atomic<float> *holdMsParam = nullptr;
    std::atomic<float> *gridMinDbParam = nullptr;
    std::atomic<float> *gridMaxDbParam = nullptr;
    std::atomic<float> *gridStepDbParam = nullptr;

    std::optional<ParameterState> previousParameterState;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void cacheParameterPointers();
    ParameterState readCurrentParameterState() const;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessor)
};
