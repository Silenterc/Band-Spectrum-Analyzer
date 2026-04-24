#include "PluginStateSerializer.h"

#include "plugin/parameters/ParameterSchema.h"

namespace {
    constexpr auto parameterNodeType = "PARAM";
    constexpr auto parameterIdProperty = "id";

    juce::String getParameterId(const juce::ValueTree& child) {
        return child.getProperty(parameterIdProperty).toString();
    }

    bool isParameterChild(const juce::ValueTree& child) {
        return child.getType() == juce::Identifier(parameterNodeType);
    }

    bool containsParameterChild(const juce::ValueTree& state, const juce::String& parameterId) {
        for (const auto& child : state) {
            if (isParameterChild(child) && getParameterId(child) == parameterId)
                return true;
        }

        return false;
    }

    void removeUiScaleChild(juce::ValueTree& state) {
        for (int childIndex = state.getNumChildren(); --childIndex >= 0;) {
            const auto child = state.getChild(childIndex);
            if (isParameterChild(child) && getParameterId(child) == PluginParameters::Schema::uiScaleId) {
                state.removeChild(childIndex, nullptr);
            }
        }
    }

    void copyMissingProperties(const juce::ValueTree& source, juce::ValueTree& destination) {
        for (int propertyIndex = 0; propertyIndex < source.getNumProperties(); ++propertyIndex) {
            const auto propertyName = source.getPropertyName(propertyIndex);
            if (propertyName == juce::Identifier(PluginParameters::Schema::uiScaleId) || destination.hasProperty(propertyName))
                continue;

            destination.setProperty(propertyName, source.getProperty(propertyName), nullptr);
        }
    }

    bool statesAreEquivalent(const juce::ValueTree& lhs, const juce::ValueTree& rhs) {
        if (!lhs.isValid() || !rhs.isValid())
            return lhs.isValid() == rhs.isValid();

        if (lhs.getType() != rhs.getType() || lhs.getNumProperties() != rhs.getNumProperties()
            || lhs.getNumChildren() != rhs.getNumChildren()) {
            return false;
        }

        for (int propertyIndex = 0; propertyIndex < lhs.getNumProperties(); ++propertyIndex) {
            const auto propertyName = lhs.getPropertyName(propertyIndex);
            if (!rhs.hasProperty(propertyName))
                return false;

            if (lhs.getProperty(propertyName).toString() != rhs.getProperty(propertyName).toString())
                return false;
        }

        for (int childIndex = 0; childIndex < lhs.getNumChildren(); ++childIndex) {
            if (!statesAreEquivalent(lhs.getChild(childIndex), rhs.getChild(childIndex)))
                return false;
        }

        return true;
    }
}

PluginPresets::PluginStateSnapshot PluginStateSerializer::captureState(
    const juce::AudioProcessorValueTreeState& parameters,
    const SignalSlotOrderState& signalSlotOrderState) const {
    auto& mutableParameters = const_cast<juce::AudioProcessorValueTreeState&>(parameters);
    auto state = mutableParameters.copyState();
    state.removeProperty(juce::Identifier(PluginParameters::Schema::uiScaleId), nullptr);
    removeUiScaleChild(state);
    signalSlotOrderState.writeTo(state);
    return {state};
}

bool PluginStateSerializer::applyState(const PluginPresets::PluginStateSnapshot& snapshot,
                                       juce::AudioProcessorValueTreeState& parameters,
                                       SignalSlotOrderState& signalSlotOrderState) const {
    if (!snapshot.isValid())
        return false;

    auto mergedState = mergeSnapshotIntoCurrentState(normaliseSnapshotState(snapshot.state.createCopy()),
                                                     normaliseSnapshotState(parameters.copyState()));
    parameters.replaceState(mergedState);
    signalSlotOrderState.readFrom(mergedState);
    return true;
}

bool PluginStateSerializer::statesEqual(const PluginPresets::PluginStateSnapshot& lhs,
                                        const PluginPresets::PluginStateSnapshot& rhs) const {
    if (!lhs.isValid() || !rhs.isValid())
        return lhs.isValid() == rhs.isValid();

    const auto normalisedLhs = normaliseSnapshotState(lhs.state.createCopy());
    const auto normalisedRhs = normaliseSnapshotState(rhs.state.createCopy());
    const auto migratedLhs = mergeSnapshotIntoCurrentState(normalisedLhs, normalisedRhs);
    const auto migratedRhs = mergeSnapshotIntoCurrentState(normalisedRhs, normalisedLhs);
    return statesAreEquivalent(migratedLhs, migratedRhs);
}

juce::ValueTree PluginStateSerializer::normaliseSnapshotState(juce::ValueTree state) const {
    state.removeProperty(juce::Identifier(PluginParameters::Schema::uiScaleId), nullptr);
    removeUiScaleChild(state);
    return state;
}

juce::ValueTree PluginStateSerializer::mergeSnapshotIntoCurrentState(const juce::ValueTree& snapshotState,
                                                                     const juce::ValueTree& currentState) const {
    auto mergedState = snapshotState.createCopy();
    copyMissingProperties(currentState, mergedState);

    for (const auto& child : currentState) {
        if (!isParameterChild(child))
            continue;

        const auto parameterId = getParameterId(child);
        if (parameterId == PluginParameters::Schema::uiScaleId || !containsParameterChild(snapshotState, parameterId))
            mergedState.addChild(child.createCopy(), -1, nullptr);
    }

    return mergedState;
}
