#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../dsp/EngineParameterState.h"
#include "../ui/AnalyzerDataSource.h"
#include "../ui/AnalyzerSettingsActions.h"
#include "../ui/AnalyzerUiStateSource.h"
#include "ParamIDs.h"
#include "ParamSpec.h"
#include "SignalSlotOrderState.h"
#include "../dsp/AnalyzerEngine.h"

//==============================================================================
class SpectrumAnalyzerAudioProcessor final : public juce::AudioProcessor,
                                             public AnalyzerDataSource,
                                             public AnalyzerSettingsActions,
                                             public AnalyzerUiStateSource,
                                             private juce::ValueTree::Listener,
                                             private SignalSlotOrderState::Listener,
                                             private juce::AsyncUpdater {
public:
    //==============================================================================
    SpectrumAnalyzerAudioProcessor();

    ~SpectrumAnalyzerAudioProcessor() override;

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

    Ui::AnalyzerUiState getAnalyzerUiState() const override;
    void addAnalyzerUiStateListener(AnalyzerUiStateSource::Listener &listener) override;
    void removeAnalyzerUiStateListener(AnalyzerUiStateSource::Listener &listener) override;

    std::shared_ptr<const std::vector<Analyzer::BandInfo>> getBandInfo() const override;
    std::vector<Analyzer::RawTrace> getRawTraces() const override;
    std::array<Ui::SignalSlotState, Shared::maxSignalSlots> getSignalSlots() const override;
    Shared::SignalSlotOrder getSignalSlotOrder() const override;
    bool isSidechainAvailable() const override;
    bool hasRecentSignal() const override;
    bool isFrozen() const override;
    Analyzer::MeterSettings getMeterSettings() const override;

    float getGridMinDb() const override;

    float getGridMaxDb() const override;

    float getGridStepDb() const override;

    void setFreezeEnabled(bool isFrozen) override;
    void setSignalSlotEnabled(size_t slotIndex, bool isEnabled) override;
    void setSignalSlotVisible(size_t slotIndex, bool isVisible) override;
    void setSignalSlotSource(size_t slotIndex, Analyzer::SignalSource source) override;
    void setSignalSlotMode(size_t slotIndex, Analyzer::SignalMode mode) override;
    void setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) override;
    void setSignalSlotColour(size_t slotIndex, int colourIndex) override;
    void setSignalSlotOpacity(size_t slotIndex, float opacity) override;
    void setShowPeakEnabled(bool isEnabled) override;
    void setShowRmsEnabled(bool isEnabled) override;
    void setShowHoldEnabled(bool isEnabled) override;

private:
    struct SignalSlotParameterPointers {
        std::atomic<float> *enabled = nullptr;
        std::atomic<float> *visible = nullptr;
        std::atomic<float> *source = nullptr;
        std::atomic<float> *mode = nullptr;
        std::atomic<float> *colour = nullptr;
        std::atomic<float> *opacity = nullptr;
    };

    Analyzer::Engine engine;
    juce::AudioProcessorValueTreeState parameters;
    SignalSlotOrderState signalSlotOrderState;

    std::atomic<float> *bandModeParam = nullptr;
    std::atomic<float> *freezeParam = nullptr;
    std::array<SignalSlotParameterPointers, Shared::maxSignalSlots> signalSlotParams{};

    std::atomic<float> *showRmsParam = nullptr;
    std::atomic<float> *showPeakParam = nullptr;
    std::atomic<float> *showHoldParam = nullptr;

    std::atomic<float> *holdMsParam = nullptr;
    std::atomic<float> *gridMinDbParam = nullptr;
    std::atomic<float> *gridMaxDbParam = nullptr;
    std::atomic<float> *gridStepDbParam = nullptr;

    std::optional<Analyzer::EngineParameterState> previousEngineParameters;
    std::optional<Ui::AnalyzerUiState> lastPublishedUiState;
    juce::ListenerList<AnalyzerUiStateSource::Listener> uiStateListeners;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void cacheParameterPointers();
    void registerUiStateParameterListeners();
    void unregisterUiStateParameterListeners();
    void triggerUiStateUpdate();
    void setBoolParameter(const juce::String &parameterID, bool value);
    void setChoiceParameter(const juce::String &parameterID, int choiceIndex, int choiceCount);
    void setFloatParameter(const juce::String &parameterID, float plainValue);
    Analyzer::EngineParameterState readCurrentEngineParameters() const;
    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;
    void valueTreeChildAdded(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenAdded) override;
    void valueTreeChildRemoved(juce::ValueTree &parentTree,
                               juce::ValueTree &childWhichHasBeenRemoved,
                               int indexFromWhichChildWasRemoved) override;
    void valueTreeChildOrderChanged(juce::ValueTree &parentTreeWhoseChildrenHaveMoved,
                                    int oldIndex,
                                    int newIndex) override;
    void valueTreeParentChanged(juce::ValueTree &treeWhoseParentHasChanged) override;
    void signalSlotOrderChanged(const Shared::SignalSlotOrder &slotOrder) override;
    void handleAsyncUpdate() override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessor)
};
