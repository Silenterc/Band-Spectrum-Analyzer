#include "ProcessorChangeTracker.h"

namespace {
    template<typename Fn>
    void forEachEngineParameterId(Fn &&fn) {
        fn(PluginParameters::Schema::bandModeId);

        for (size_t slotIndex = 0; slotIndex < Shared::maxSignalSlots; ++slotIndex) {
            fn(PluginParameters::Schema::slotParameterId(PluginParameters::Schema::SlotField::enabled, slotIndex));
            fn(PluginParameters::Schema::slotParameterId(PluginParameters::Schema::SlotField::source, slotIndex));
            fn(PluginParameters::Schema::slotParameterId(PluginParameters::Schema::SlotField::mode, slotIndex));
        }
    }
}

ProcessorChangeTracker::ProcessorChangeTracker(juce::AudioProcessorValueTreeState &parametersToUse,
                                               SignalSlotOrderState &signalSlotOrderStateToUse,
                                               Listener &listenerToUse)
    : parameters(parametersToUse),
      signalSlotOrderState(signalSlotOrderStateToUse),
      listener(listenerToUse) {
    attach();
}

ProcessorChangeTracker::~ProcessorChangeTracker() {
    detach();
    cancelPendingUpdate();
}

bool ProcessorChangeTracker::consumeEngineParametersDirty() {
    return engineParametersDirty.exchange(false, std::memory_order_acq_rel);
}

void ProcessorChangeTracker::clearEngineParametersDirty() {
    engineParametersDirty.store(false, std::memory_order_release);
}

void ProcessorChangeTracker::requestUiRefresh() {
    if (const auto *messageManager = juce::MessageManager::getInstanceWithoutCreating();
        messageManager != nullptr && messageManager->isThisTheMessageThread()) {
        handleAsyncUpdate();
        return;
    }

    triggerAsyncUpdate();
}

void ProcessorChangeTracker::attach() {
    parameters.state.addListener(this);
    signalSlotOrderState.addListener(*this);
    forEachEngineParameterId([this](const juce::String &parameterId) {
        parameters.addParameterListener(parameterId, this);
    });
}

void ProcessorChangeTracker::detach() {
    forEachEngineParameterId([this](const juce::String &parameterId) {
        parameters.removeParameterListener(parameterId, this);
    });
    signalSlotOrderState.removeListener(*this);
    parameters.state.removeListener(this);
}

void ProcessorChangeTracker::parameterChanged(const juce::String &parameterID, const float newValue) {
    juce::ignoreUnused(parameterID, newValue);
    engineParametersDirty.store(true, std::memory_order_release);
    listener.processorPresetStateChanged();
}

void ProcessorChangeTracker::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                      const juce::Identifier &property) {
    juce::ignoreUnused(treeWhosePropertyHasChanged, property);
    listener.processorPresetStateChanged();
    requestUiRefresh();
}

void ProcessorChangeTracker::valueTreeChildAdded(juce::ValueTree &parentTree,
                                                 juce::ValueTree &childWhichHasBeenAdded) {
    juce::ignoreUnused(parentTree, childWhichHasBeenAdded);
    listener.processorPresetStateChanged();
    requestUiRefresh();
}

void ProcessorChangeTracker::valueTreeChildRemoved(juce::ValueTree &parentTree,
                                                   juce::ValueTree &childWhichHasBeenRemoved,
                                                   const int indexFromWhichChildWasRemoved) {
    juce::ignoreUnused(parentTree, childWhichHasBeenRemoved, indexFromWhichChildWasRemoved);
    listener.processorPresetStateChanged();
    requestUiRefresh();
}

void ProcessorChangeTracker::valueTreeChildOrderChanged(juce::ValueTree &parentTreeWhoseChildrenHaveMoved,
                                                        const int oldIndex,
                                                        const int newIndex) {
    juce::ignoreUnused(parentTreeWhoseChildrenHaveMoved, oldIndex, newIndex);
    listener.processorPresetStateChanged();
    requestUiRefresh();
}

void ProcessorChangeTracker::valueTreeParentChanged(juce::ValueTree &treeWhoseParentHasChanged) {
    juce::ignoreUnused(treeWhoseParentHasChanged);
    listener.processorPresetStateChanged();
    requestUiRefresh();
}

void ProcessorChangeTracker::signalSlotOrderChanged() {
    listener.processorPresetStateChanged();
    requestUiRefresh();
}

void ProcessorChangeTracker::handleAsyncUpdate() {
    listener.processorUiRefreshRequested();
}
