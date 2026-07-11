#pragma once

#include <memory>
#include <optional>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../dsp/core/AnalyzerEngine.h"
#include "../dsp/core/EngineParameterState.h"
#include "PluginUiBridge.h"
#include "ProcessorChangeTracker.h"
#include "SignalOutputMixer.h"
#include "parameters/ParameterAccess.h"
#include "parameters/ParameterSchema.h"
#include "presets/FactoryPresetRepository.h"
#include "presets/PluginStateSerializer.h"
#include "presets/PresetSession.h"
#include "presets/UserPresetStore.h"
#include "state/SignalSlotOrderState.h"

//==============================================================================
class SpectrumAnalyzerAudioProcessor final : public juce::AudioProcessor {
public:
    //==============================================================================
    SpectrumAnalyzerAudioProcessor();

    ~SpectrumAnalyzerAudioProcessor() override = default;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;
    void numBusesChanged() override;
    void processorLayoutsChanged() override;

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
    bool isSidechainAvailable() const;

    Analyzer::Engine engine;
    SignalOutputMixer outputMixer;
    juce::AudioProcessorValueTreeState parameters;
    PluginParameters::Access parameterAccess;
    SignalSlotOrderState signalSlotOrderState;
    PluginStateSerializer pluginStateSerializer;
    UserPresetStore userPresetStore;
    FactoryPresetRepository factoryPresetRepository;
    PresetSession presetSession;
    PluginUiBridge uiBridge;
    std::unique_ptr<ProcessorChangeTracker> changeTracker;

    std::optional<Analyzer::EngineParameterState> previousEngineParameters;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessor)
};
