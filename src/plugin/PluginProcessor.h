#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../dsp/core/EngineParameterState.h"
#include "../ui/AnalyzerRenderSource.h"
#include "../ui/AnalyzerSettingsActions.h"
#include "../ui/AnalyzerUiSnapshotSource.h"
#include "../dsp/core/AnalyzerEngine.h"
#include "parameters/ParameterAccess.h"
#include "parameters/ParameterSchema.h"
#include "state/SignalSlotOrderState.h"

//==============================================================================
class SpectrumAnalyzerAudioProcessor final : public juce::AudioProcessor,
                                             public AnalyzerRenderSource,
                                             public AnalyzerSettingsActions,
                                             public AnalyzerUiSnapshotSource,
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

    Ui::AnalyzerUiSnapshot getAnalyzerUiSnapshot() const override;
    void addAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) override;
    void removeAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) override;

    std::shared_ptr<const std::vector<Analyzer::BandInfo>> getBandInfo() const override;
    std::vector<Analyzer::RawTrace> getRawTraces() const override;
    bool hasRecentSignal() const override;

    void setFreezeEnabled(bool isFrozen) override;
    void setSignalSlotEnabled(size_t slotIndex, bool isEnabled) override;
    void setSignalSlotVisible(size_t slotIndex, bool isVisible) override;
    void setSignalSlotFrozen(size_t slotIndex, bool isFrozen) override;
    void setSignalSlotSource(size_t slotIndex, Analyzer::SignalSource source) override;
    void setSignalSlotMode(size_t slotIndex, Analyzer::SignalMode mode) override;
    void setSignalSlotSignal(size_t slotIndex, Analyzer::SignalSource source, Analyzer::SignalMode mode) override;
    void applySignalSlotState(size_t slotIndex, const Ui::SignalSlotState &state) override;
    void removeSignalSlot(size_t slotIndex) override;
    void addSignalSlot(size_t slotIndex,
                       const Ui::SignalSlotState &state,
                       const Shared::SignalSlotOrder &slotOrder) override;
    void setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) override;
    void setSignalSlotColour(size_t slotIndex, int colourIndex) override;
    void setSignalSlotOpacity(size_t slotIndex, float opacity) override;
    void setShowPeakEnabled(bool isEnabled) override;
    void setShowRmsEnabled(bool isEnabled) override;
    void setShowHoldEnabled(bool isEnabled) override;

private:
    std::array<Ui::SignalSlotState, Shared::maxSignalSlots> getSignalSlots() const;
    Shared::SignalSlotOrder getSignalSlotOrder() const;
    bool isSidechainAvailable() const;
    bool isFrozen() const;
    Analyzer::MeterSettings getMeterSettings() const;
    float getGridMinDb() const;
    float getGridMaxDb() const;
    float getGridStepDb() const;

    Analyzer::Engine engine;
    juce::AudioProcessorValueTreeState parameters;
    PluginParameters::Access parameterAccess;
    SignalSlotOrderState signalSlotOrderState;

    std::optional<Analyzer::EngineParameterState> previousEngineParameters;
    std::optional<Ui::AnalyzerUiSnapshot> lastPublishedUiSnapshot;
    juce::ListenerList<AnalyzerUiSnapshotSource::Listener> uiSnapshotListeners;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void registerUiStateParameterListeners();
    void unregisterUiStateParameterListeners();
    void triggerUiStateUpdate();
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
