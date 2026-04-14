#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../display/analyzer/contracts/AnalyzerRawTraceSource.h"
#include "../dsp/core/EngineParameterState.h"
#include "../ui/contracts/AnalyzerSettingsActions.h"
#include "../ui/contracts/AnalyzerUiSnapshotSource.h"
#include "../dsp/core/AnalyzerEngine.h"
#include "ProcessorChangeTracker.h"
#include "parameters/ParameterAccess.h"
#include "parameters/ParameterSchema.h"
#include "state/SignalSlotOrderState.h"

//==============================================================================
class SpectrumAnalyzerAudioProcessor final : public juce::AudioProcessor,
                                             public AnalyzerRawTraceSource,
                                             public AnalyzerSettingsActions,
                                             public AnalyzerUiSnapshotSource,
                                             private ProcessorChangeTracker::Listener {
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

    // AnalyzerUiSnapshotSource
    Ui::AnalyzerUiSnapshot getAnalyzerUiSnapshot() const override;
    void addAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) override;
    void removeAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) override;

    // AnalyzerRawTraceSource
    std::shared_ptr<const std::vector<Analyzer::BandInfo>> getBandInfo() const override;
    AnalyzerPublishedTracesView readPublishedTraces() const override;
    bool hasRecentSignal() const override;
    bool shouldProcessAnalyzer() const override;

    // AnalyzerSettingsActions
    void setFreezeEnabled(bool isFrozen) override;
    void setSignalSlotEnabled(size_t slotIndex, bool isEnabled) override;
    void setSignalSlotVisible(size_t slotIndex, bool isVisible) override;
    void setSignalSlotFrozen(size_t slotIndex, bool isFrozen) override;
    void setSignalSlotSource(size_t slotIndex, Analyzer::SignalSource source) override;
    void setSignalSlotMode(size_t slotIndex, Analyzer::SignalMode mode) override;
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
    void setCustomFrequencyRangeEnabled(bool isEnabled) override;
    void setVisibleMinFrequencyHz(float frequencyHz) override;
    void setVisibleMaxFrequencyHz(float frequencyHz) override;

private:
    std::array<Ui::SignalSlotState, Shared::maxSignalSlots> getSignalSlots() const;
    Shared::SignalSlotOrder getSignalSlotOrder() const;
    bool isSidechainAvailable() const;
    bool isFrozen() const;
    Analyzer::MeterSettings getMeterSettings() const;
    float getGridMinDb() const;
    float getGridMaxDb() const;
    float getGridStepDb() const;
    bool getUseCustomFrequencyRange() const;
    float getVisibleMinFrequencyHz() const;
    float getVisibleMaxFrequencyHz() const;

    Analyzer::Engine engine;
    juce::AudioProcessorValueTreeState parameters;
    PluginParameters::Access parameterAccess;
    SignalSlotOrderState signalSlotOrderState;
    std::unique_ptr<ProcessorChangeTracker> changeTracker;

    std::optional<Analyzer::EngineParameterState> previousEngineParameters;
    std::optional<Ui::AnalyzerUiSnapshot> lastPublishedUiSnapshot;
    juce::ListenerList<AnalyzerUiSnapshotSource::Listener> uiSnapshotListeners;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void processorUiRefreshRequested() override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessor)
};
