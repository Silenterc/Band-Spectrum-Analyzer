#include "ParameterAccess.h"

namespace PluginParameters {
    Access::Access(juce::AudioProcessorValueTreeState &parametersToUse)
        : parameters(parametersToUse) {
    }

    void Access::cache() {
        bandModeParam = parameters.getRawParameterValue(Schema::bandModeId);
        freezeParam = parameters.getRawParameterValue(Schema::freezeId);

        for (size_t slotIndex = 0; slotIndex < slotParams.size(); ++slotIndex) {
            auto &slot = slotParams[slotIndex];
            slot.enabled = parameters.getRawParameterValue(Schema::slotParameterId(Schema::SlotField::enabled, slotIndex));
            slot.solo = parameters.getRawParameterValue(Schema::slotParameterId(Schema::SlotField::solo, slotIndex));
            slot.visible = parameters.getRawParameterValue(Schema::slotParameterId(Schema::SlotField::visible, slotIndex));
            slot.frozen = parameters.getRawParameterValue(Schema::slotParameterId(Schema::SlotField::frozen, slotIndex));
            slot.source = parameters.getRawParameterValue(Schema::slotParameterId(Schema::SlotField::source, slotIndex));
            slot.mode = parameters.getRawParameterValue(Schema::slotParameterId(Schema::SlotField::mode, slotIndex));
            slot.colour = parameters.getRawParameterValue(Schema::slotParameterId(Schema::SlotField::colour, slotIndex));
            slot.opacity = parameters.getRawParameterValue(Schema::slotParameterId(Schema::SlotField::opacity, slotIndex));
        }

        showRmsParam = parameters.getRawParameterValue(Schema::showRmsId);
        showPeakParam = parameters.getRawParameterValue(Schema::showPeakId);
        showHoldParam = parameters.getRawParameterValue(Schema::showHoldId);
        holdMsParam = parameters.getRawParameterValue(Schema::holdMsId);
        rmsWindowMsParam = parameters.getRawParameterValue(Schema::rmsWindowMsId);
        peakDecayDbPerSecondParam = parameters.getRawParameterValue(Schema::peakDecayDbPerSecondId);
        holdDecayDbPerSecondParam = parameters.getRawParameterValue(Schema::holdDecayDbPerSecondId);
        gridMinDbParam = parameters.getRawParameterValue(Schema::gridMinDbId);
        gridMaxDbParam = parameters.getRawParameterValue(Schema::gridMaxDbId);
        gridStepDbParam = parameters.getRawParameterValue(Schema::gridStepDbId);
        useCustomFrequencyRangeParam = parameters.getRawParameterValue(Schema::useCustomFrequencyRangeId);
        visibleMinFrequencyHzParam = parameters.getRawParameterValue(Schema::visibleMinFrequencyHzId);
        visibleMaxFrequencyHzParam = parameters.getRawParameterValue(Schema::visibleMaxFrequencyHzId);
        uiScaleParam = parameters.getRawParameterValue(Schema::uiScaleId);
    }

    Analyzer::EngineParameterState Access::readEngineState() const {
        Analyzer::EngineParameterState state;
        state.bandMode = readBandMode();

        for (size_t slotIndex = 0; slotIndex < slotParams.size(); ++slotIndex) {
            const auto &slot = slotParams[slotIndex];
            auto &configuration = state.signalSlots[slotIndex];
            configuration.enabled = readBool(slot.enabled);
            configuration.source = readChoice(slot.source, Schema::signalSourceChoices);
            configuration.mode = readChoice(slot.mode, Schema::signalModeChoices);
        }

        return state;
    }

    std::array<SignalOutputMixer::SlotState, Shared::maxSignalSlots> Access::readMixerSlots() const {
        std::array<SignalOutputMixer::SlotState, Shared::maxSignalSlots> slots{};

        for (size_t slotIndex = 0; slotIndex < slots.size(); ++slotIndex) {
            const auto &parameterRefs = slotParams[slotIndex];
            auto &slot = slots[slotIndex];
            slot.enabled = readBool(parameterRefs.enabled);
            slot.solo = readBool(parameterRefs.solo);
            slot.source = readChoice(parameterRefs.source, Schema::signalSourceChoices);
            slot.mode = readChoice(parameterRefs.mode, Schema::signalModeChoices);
        }

        return slots;
    }

    std::array<Ui::SignalSlotState, Shared::maxSignalSlots> Access::readUiSlots() const {
        std::array<Ui::SignalSlotState, Shared::maxSignalSlots> slots{};

        for (size_t slotIndex = 0; slotIndex < slots.size(); ++slotIndex)
            slots[slotIndex] = readUiSlot(slotIndex);

        return slots;
    }

    Ui::SignalSlotState Access::readUiSlot(const size_t slotIndex) const {
        jassert(slotIndex < slotParams.size());
        const auto &slot = slotParams[slotIndex];

        Ui::SignalSlotState state;
        state.configuration.enabled = readBool(slot.enabled);
        state.solo = readBool(slot.solo);
        state.visible = readBool(slot.visible);
        state.frozen = readBool(slot.frozen);
        state.configuration.source = readChoice(slot.source, Schema::signalSourceChoices);
        state.configuration.mode = readChoice(slot.mode, Schema::signalModeChoices);
        state.colourIndex = juce::jlimit(0, Shared::signalPresetCount - 1, juce::roundToInt(slot.colour->load()));
        state.opacity = readFloat(slot.opacity);
        return state;
    }

    Analyzer::MeterSettings Access::readMeterSettings() const {
        Analyzer::MeterSettings settings;
        settings.showRms = readBool(showRmsParam);
        settings.showPeak = readBool(showPeakParam);
        settings.showHold = readBool(showHoldParam);
        settings.holdMs = readFloat(holdMsParam);
        settings.rmsWindowMs = readFloat(rmsWindowMsParam);
        settings.peakDecayDbPerSecond = readFloat(peakDecayDbPerSecondParam);
        settings.holdDecayDbPerSecond = readFloat(holdDecayDbPerSecondParam);
        return settings;
    }

    Analyzer::BandMode Access::readBandMode() const {
        return readChoice(bandModeParam, Schema::bandModeChoices);
    }

    bool Access::readFreeze() const {
        return readBool(freezeParam);
    }

    float Access::readGridMinDb() const {
        return readFloat(gridMinDbParam);
    }

    float Access::readGridMaxDb() const {
        return readFloat(gridMaxDbParam);
    }

    float Access::readGridStepDb() const {
        return readFloat(gridStepDbParam);
    }

    bool Access::readUseCustomFrequencyRange() const {
        return readBool(useCustomFrequencyRangeParam);
    }

    float Access::readVisibleMinFrequencyHz() const {
        return readFloat(visibleMinFrequencyHzParam);
    }

    float Access::readVisibleMaxFrequencyHz() const {
        return readFloat(visibleMaxFrequencyHzParam);
    }

    Ui::UiScalePreset Access::readUiScalePreset() const {
        return readChoice(uiScaleParam, Schema::uiScaleChoices);
    }

    void Access::writeFreeze(const bool value) {
        writeBool(Schema::freezeId, value);
    }

    void Access::writeSlotState(const size_t slotIndex, const Ui::SignalSlotState &state) {
        writeSlotSignal(slotIndex, state.configuration.source, state.configuration.mode);
        writeSlotColour(slotIndex, state.colourIndex);
        writeSlotOpacity(slotIndex, state.opacity);
        writeSlotSolo(slotIndex, state.solo);
        writeSlotVisible(slotIndex, state.visible);
        writeSlotFrozen(slotIndex, state.frozen);
        writeSlotEnabled(slotIndex, state.configuration.enabled);
    }

    void Access::writeSlotSignal(const size_t slotIndex, const Analyzer::SignalSource source, const Analyzer::SignalMode mode) {
        writeFloat(Schema::slotParameterId(Schema::SlotField::source, slotIndex),
                   static_cast<float>(Schema::indexForValue(source, Schema::signalSourceChoices)));
        writeFloat(Schema::slotParameterId(Schema::SlotField::mode, slotIndex),
                   static_cast<float>(Schema::indexForValue(mode, Schema::signalModeChoices)));
    }

    void Access::writeSlotEnabled(const size_t slotIndex, const bool value) {
        writeBool(Schema::slotParameterId(Schema::SlotField::enabled, slotIndex), value);
    }

    void Access::writeSlotSolo(const size_t slotIndex, const bool value) {
        writeBool(Schema::slotParameterId(Schema::SlotField::solo, slotIndex), value);
    }

    void Access::writeSlotVisible(const size_t slotIndex, const bool value) {
        writeBool(Schema::slotParameterId(Schema::SlotField::visible, slotIndex), value);
    }

    void Access::writeSlotFrozen(const size_t slotIndex, const bool value) {
        writeBool(Schema::slotParameterId(Schema::SlotField::frozen, slotIndex), value);
    }

    void Access::writeSlotColour(const size_t slotIndex, const int colourIndex) {
        writeFloat(Schema::slotParameterId(Schema::SlotField::colour, slotIndex),
                   static_cast<float>(juce::jlimit(0, Shared::signalPresetCount - 1, colourIndex)));
    }

    void Access::writeSlotOpacity(const size_t slotIndex, const float opacity) {
        writeFloat(Schema::slotParameterId(Schema::SlotField::opacity, slotIndex),
                   juce::jlimit(Defaults::signalOpacityMin, Defaults::signalOpacityMax, opacity));
    }

    void Access::writeShowPeak(const bool value) {
        writeBool(Schema::showPeakId, value);
    }

    void Access::writeShowRms(const bool value) {
        writeBool(Schema::showRmsId, value);
    }

    void Access::writeShowHold(const bool value) {
        writeBool(Schema::showHoldId, value);
    }

    void Access::writeUseCustomFrequencyRange(const bool value) {
        writeBool(Schema::useCustomFrequencyRangeId, value);
    }

    void Access::writeVisibleMinFrequencyHz(const float frequencyHz) {
        writeFloat(Schema::visibleMinFrequencyHzId,
                   juce::jlimit(Defaults::visibleMinFrequencyHzMin,
                                Defaults::visibleMinFrequencyHzMax,
                                frequencyHz));
    }

    void Access::writeVisibleMaxFrequencyHz(const float frequencyHz) {
        writeFloat(Schema::visibleMaxFrequencyHzId,
                   juce::jlimit(Defaults::visibleMaxFrequencyHzMin,
                                Defaults::visibleMaxFrequencyHzMax,
                                frequencyHz));
    }

    bool Access::readBool(std::atomic<float> *parameter) {
        jassert(parameter != nullptr);
        return parameter->load() > 0.5f;
    }

    float Access::readFloat(std::atomic<float> *parameter) {
        jassert(parameter != nullptr);
        return parameter->load();
    }

    void Access::writeBool(const juce::String &parameterId, const bool value) {
        if (auto *parameter = parameters.getParameter(parameterId)) {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(value ? 1.0f : 0.0f);
            parameter->endChangeGesture();
        }
    }

    void Access::writeFloat(const juce::String &parameterId, const float plainValue) {
        if (auto *parameter = parameters.getParameter(parameterId)) {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->convertTo0to1(plainValue));
            parameter->endChangeGesture();
        }
    }
}
