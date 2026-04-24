#pragma once

#include <atomic>

#include <juce_audio_processors/juce_audio_processors.h>

#include "parameters/ParameterSchema.h"
#include "state/SignalSlotOrderState.h"

class ProcessorChangeTracker final : private juce::AudioProcessorValueTreeState::Listener,
                                     private juce::ValueTree::Listener,
                                     private SignalSlotOrderState::Listener,
                                     private juce::AsyncUpdater {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void processorPresetStateChanged() = 0;
        virtual void processorUiRefreshRequested() = 0;
    };

    ProcessorChangeTracker(juce::AudioProcessorValueTreeState &parametersToUse,
                           SignalSlotOrderState &signalSlotOrderStateToUse,
                           Listener &listenerToUse);
    ~ProcessorChangeTracker() override;

    bool consumeEngineParametersDirty();
    void clearEngineParametersDirty();
    void requestUiRefresh();

private:
    void attach();
    void detach();

    void parameterChanged(const juce::String &parameterID, float newValue) override;
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
    void signalSlotOrderChanged() override;
    void handleAsyncUpdate() override;

    juce::AudioProcessorValueTreeState &parameters;
    SignalSlotOrderState &signalSlotOrderState;
    Listener &listener;
    std::atomic<bool> engineParametersDirty { true };
};
