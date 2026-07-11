#pragma once

#include <array>
#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../../dsp/core/BandMode.h"
#include "../../shared/DefaultParameterValues.h"
#include "../../shared/ParameterOptionCatalog.h"
#include "../../shared/SignalPresetCatalog.h"
#include "../../shared/UiScalePreset.h"
#include "../../ui/analyzer/rack/state/SignalSlotUiState.h"

namespace PluginParameters::Schema {
    inline constexpr int parameterVersionHint = 1;

    enum class SlotField {
        enabled,
        solo,
        visible,
        frozen,
        source,
        mode,
        colour,
        opacity
    };

    inline constexpr auto bandModeName = "Band Mode";
    inline constexpr auto freezeName = "Freeze";
    inline constexpr auto showRmsName = "Show RMS";
    inline constexpr auto showPeakName = "Show Peak";
    inline constexpr auto showHoldName = "Show Hold";
    inline constexpr auto holdMsName = "Hold Time";
    inline constexpr auto rmsWindowMsName = "RMS Time";
    inline constexpr auto peakDecayDbPerSecondName = "Peak Decay";
    inline constexpr auto holdDecayDbPerSecondName = "Hold Decay";
    inline constexpr auto gridMinDbName = "Grid Min dB";
    inline constexpr auto gridMaxDbName = "Grid Max dB";
    inline constexpr auto gridStepDbName = "Grid Step dB";
    inline constexpr auto useCustomFrequencyRangeName = "Use Custom Frequency Range";
    inline constexpr auto visibleMinFrequencyHzName = "Visible Min Frequency";
    inline constexpr auto visibleMaxFrequencyHzName = "Visible Max Frequency";
    inline constexpr auto uiScaleName = "UI Scale";

    inline constexpr auto bandModeId = "bandMode";
    inline constexpr auto freezeId = "freeze";
    inline constexpr auto showRmsId = "showRms";
    inline constexpr auto showPeakId = "showPeak";
    inline constexpr auto showHoldId = "showHold";
    inline constexpr auto holdMsId = "holdMs";
    inline constexpr auto rmsWindowMsId = "rmsWindowMs";
    inline constexpr auto peakDecayDbPerSecondId = "peakDecayDbPerSecond";
    inline constexpr auto holdDecayDbPerSecondId = "holdDecayDbPerSecond";
    inline constexpr auto gridMinDbId = "gridMinDb";
    inline constexpr auto gridMaxDbId = "gridMaxDb";
    inline constexpr auto gridStepDbId = "gridStepDb";
    inline constexpr auto useCustomFrequencyRangeId = "useCustomFrequencyRange";
    inline constexpr auto visibleMinFrequencyHzId = "visibleMinFrequencyHz";
    inline constexpr auto visibleMaxFrequencyHzId = "visibleMaxFrequencyHz";
    inline constexpr auto uiScaleId = "uiScale";

    inline constexpr std::array<Shared::EnumChoice<Analyzer::SignalSource>, 2> signalSourceChoices{{
        {Analyzer::SignalSource::main, "Main"},
        {Analyzer::SignalSource::sidechain, "Sidechain"}
    }};

    inline constexpr std::array<Shared::EnumChoice<Analyzer::SignalMode>, 3> signalModeChoices{{
        {Analyzer::SignalMode::mid, "Mid"},
        {Analyzer::SignalMode::side, "Side"},
        {Analyzer::SignalMode::stereo, "Stereo"}
    }};

    template<typename Enum, size_t Size>
    juce::StringArray toStringArray(const std::array<Shared::EnumChoice<Enum>, Size> &choices) {
        juce::StringArray result;
        result.ensureStorageAllocated(static_cast<int>(choices.size()));
        for (const auto &choice: choices)
            result.add(choice.label);
        return result;
    }

    inline juce::StringArray makeSignalColourChoiceArray() {
        juce::StringArray result;
        result.ensureStorageAllocated(Shared::signalPresetCount);

        for (const auto &preset: Shared::signalPresetCatalog)
            result.add(preset.name);

        return result;
    }

    inline juce::ParameterID makeParameterID(const char *id) {
        return {id, parameterVersionHint};
    }

    inline juce::ParameterID makeParameterID(const juce::String &id) {
        return {id, parameterVersionHint};
    }

    inline auto holdMsRange() {
        return juce::NormalisableRange<float>(Defaults::holdMsMin, Defaults::holdMsMax, Defaults::holdMsStep);
    }

    inline auto rmsWindowMsRange() {
        return juce::NormalisableRange<float>(Defaults::rmsWindowMsMin,
                                              Defaults::rmsWindowMsMax,
                                              Defaults::rmsWindowMsStep);
    }

    inline auto peakDecayDbPerSecondRange() {
        return juce::NormalisableRange<float>(Defaults::peakDecayDbPerSecondMin,
                                              Defaults::peakDecayDbPerSecondMax,
                                              Defaults::peakDecayDbPerSecondStep);
    }

    inline auto holdDecayDbPerSecondRange() {
        return juce::NormalisableRange<float>(Defaults::holdDecayDbPerSecondMin,
                                              Defaults::holdDecayDbPerSecondMax,
                                              Defaults::holdDecayDbPerSecondStep);
    }

    inline auto gridMinDbRange() {
        return juce::NormalisableRange<float>(Defaults::gridMinDbMin, Defaults::gridMinDbMax, Defaults::gridMinDbStep);
    }

    inline auto gridMaxDbRange() {
        return juce::NormalisableRange<float>(Defaults::gridMaxDbMin, Defaults::gridMaxDbMax, Defaults::gridMaxDbStep);
    }

    inline auto gridStepDbRange() {
        return juce::NormalisableRange<float>(Defaults::gridStepDbMin, Defaults::gridStepDbMax, Defaults::gridStepDbStep);
    }

    inline auto visibleMinFrequencyHzRange() {
        return juce::NormalisableRange<float>(Defaults::visibleMinFrequencyHzMin,
                                              Defaults::visibleMinFrequencyHzMax,
                                              Defaults::visibleMinFrequencyHzStep);
    }

    inline auto visibleMaxFrequencyHzRange() {
        return juce::NormalisableRange<float>(Defaults::visibleMaxFrequencyHzMin,
                                              Defaults::visibleMaxFrequencyHzMax,
                                              Defaults::visibleMaxFrequencyHzStep);
    }

    inline constexpr const char *slotFieldSuffix(const SlotField field) {
        switch (field) {
            case SlotField::enabled: return "Enabled";
            case SlotField::solo: return "Solo";
            case SlotField::visible: return "Visible";
            case SlotField::frozen: return "Frozen";
            case SlotField::source: return "Source";
            case SlotField::mode: return "Mode";
            case SlotField::colour: return "Colour";
            case SlotField::opacity: return "Opacity";
        }

        return "";
    }

    inline juce::String slotParameterId(const SlotField field, const size_t slotIndex) {
        return "signalSlot" + juce::String(static_cast<int>(slotIndex)) + slotFieldSuffix(field);
    }

    inline juce::String slotParameterName(const SlotField field, const size_t slotIndex) {
        return "Signal " + juce::String(static_cast<int>(slotIndex + 1)) + " " + slotFieldSuffix(field);
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeSlotParameter(const SlotField field, const size_t slotIndex) {
        switch (field) {
            case SlotField::enabled:
                return std::make_unique<juce::AudioParameterBool>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    Defaults::isSignalSlotEnabled(slotIndex));
            case SlotField::solo:
                return std::make_unique<juce::AudioParameterBool>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    Defaults::isSignalSlotSolo(slotIndex));
            case SlotField::visible:
                return std::make_unique<juce::AudioParameterBool>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    Defaults::isSignalSlotVisible(slotIndex));
            case SlotField::frozen:
                return std::make_unique<juce::AudioParameterBool>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    Defaults::isSignalSlotFrozen(slotIndex));
            case SlotField::source:
                return std::make_unique<juce::AudioParameterChoice>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    toStringArray(signalSourceChoices),
                    Shared::indexForValue(Defaults::signalSlotSource(slotIndex), signalSourceChoices));
            case SlotField::mode:
                return std::make_unique<juce::AudioParameterChoice>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    toStringArray(signalModeChoices),
                    Shared::indexForValue(Defaults::signalSlotMode(slotIndex), signalModeChoices));
            case SlotField::colour:
                return std::make_unique<juce::AudioParameterChoice>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    makeSignalColourChoiceArray(),
                    Defaults::signalSlotColour(slotIndex));
            case SlotField::opacity:
                return std::make_unique<juce::AudioParameterFloat>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    juce::NormalisableRange<float>(Defaults::signalOpacityMin,
                                                   Defaults::signalOpacityMax,
                                                   Defaults::signalOpacityStep),
                    Defaults::signalOpacity);
        }

        return {};
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout makeParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            makeParameterID(bandModeId),
            bandModeName,
            toStringArray(Shared::bandModeChoices),
            Shared::indexForValue(Defaults::bandMode, Shared::bandModeChoices)));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            makeParameterID(freezeId),
            freezeName,
            Defaults::freeze));

        constexpr std::array slotFields{
            SlotField::enabled,
            SlotField::solo,
            SlotField::visible,
            SlotField::frozen,
            SlotField::source,
            SlotField::mode,
            SlotField::colour,
            SlotField::opacity
        };

        for (size_t slotIndex = 0; slotIndex < Shared::maxSignalSlots; ++slotIndex) {
            for (const auto field: slotFields)
                layout.add(makeSlotParameter(field, slotIndex));
        }

        layout.add(std::make_unique<juce::AudioParameterBool>(
            makeParameterID(showRmsId),
            showRmsName,
            Defaults::showRms));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            makeParameterID(showPeakId),
            showPeakName,
            Defaults::showPeak));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            makeParameterID(showHoldId),
            showHoldName,
            Defaults::showHold));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(holdMsId),
            holdMsName,
            holdMsRange(),
            Defaults::holdMs));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(rmsWindowMsId),
            rmsWindowMsName,
            rmsWindowMsRange(),
            Defaults::rmsWindowMs));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(peakDecayDbPerSecondId),
            peakDecayDbPerSecondName,
            peakDecayDbPerSecondRange(),
            Defaults::peakDecayDbPerSecond));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(holdDecayDbPerSecondId),
            holdDecayDbPerSecondName,
            holdDecayDbPerSecondRange(),
            Defaults::holdDecayDbPerSecond));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(gridMinDbId),
            gridMinDbName,
            gridMinDbRange(),
            Defaults::gridMinDb));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(gridMaxDbId),
            gridMaxDbName,
            gridMaxDbRange(),
            Defaults::gridMaxDb));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(gridStepDbId),
            gridStepDbName,
            gridStepDbRange(),
            Defaults::gridStepDb));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            makeParameterID(useCustomFrequencyRangeId),
            useCustomFrequencyRangeName,
            Defaults::useCustomFrequencyRange));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(visibleMinFrequencyHzId),
            visibleMinFrequencyHzName,
            visibleMinFrequencyHzRange(),
            Defaults::visibleMinFrequencyHz));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(visibleMaxFrequencyHzId),
            visibleMaxFrequencyHzName,
            visibleMaxFrequencyHzRange(),
            Defaults::visibleMaxFrequencyHz));
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            makeParameterID(uiScaleId),
            uiScaleName,
            toStringArray(Shared::uiScaleChoices),
            Shared::indexForValue(Defaults::uiScalePreset, Shared::uiScaleChoices)));

        return layout;
    }
}
