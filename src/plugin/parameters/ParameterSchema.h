#pragma once

#include <array>
#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../../dsp/core/BandMode.h"
#include "../../shared/DefaultParameterValues.h"
#include "../../shared/SignalPresetCatalog.h"
#include "../../ui/state/SignalSlotUiState.h"

namespace PluginParameters::Schema {
    inline constexpr int parameterVersionHint = 1;

    enum class SlotField {
        enabled,
        visible,
        frozen,
        source,
        mode,
        colour,
        opacity
    };

    template<typename Enum>
    struct EnumChoice {
        Enum value;
        const char *label;
    };

    inline constexpr auto bandModeName = "Band Mode";
    inline constexpr auto freezeName = "Freeze";
    inline constexpr auto showRmsName = "Show RMS";
    inline constexpr auto showPeakName = "Show Peak";
    inline constexpr auto showHoldName = "Show Hold";
    inline constexpr auto holdMsName = "Hold Time";
    inline constexpr auto gridMinDbName = "Grid Min dB";
    inline constexpr auto gridMaxDbName = "Grid Max dB";
    inline constexpr auto gridStepDbName = "Grid Step dB";

    inline constexpr auto bandModeId = "bandMode";
    inline constexpr auto freezeId = "freeze";
    inline constexpr auto showRmsId = "showRms";
    inline constexpr auto showPeakId = "showPeak";
    inline constexpr auto showHoldId = "showHold";
    inline constexpr auto holdMsId = "holdMs";
    inline constexpr auto gridMinDbId = "gridMinDb";
    inline constexpr auto gridMaxDbId = "gridMaxDb";
    inline constexpr auto gridStepDbId = "gridStepDb";

    inline constexpr std::array<EnumChoice<Analyzer::BandMode>, 4> bandModeChoices{{
        {Analyzer::BandMode::octaveThird, "1/3 Oct"},
        {Analyzer::BandMode::octaveQuarter, "1/4 Oct"},
        {Analyzer::BandMode::octaveSixth, "1/6 Oct"},
        {Analyzer::BandMode::octaveTwelfth, "1/12 Oct"}
    }};

    inline constexpr std::array<EnumChoice<Analyzer::SignalSource>, 2> signalSourceChoices{{
        {Analyzer::SignalSource::main, "Main"},
        {Analyzer::SignalSource::sidechain, "Sidechain"}
    }};

    inline constexpr std::array<EnumChoice<Analyzer::SignalMode>, 3> signalModeChoices{{
        {Analyzer::SignalMode::mid, "Mid"},
        {Analyzer::SignalMode::side, "Side"},
        {Analyzer::SignalMode::stereo, "Stereo"}
    }};

    template<typename Enum, size_t Size>
    juce::StringArray toStringArray(const std::array<EnumChoice<Enum>, Size> &choices) {
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

    template<typename Enum, size_t Size>
    constexpr int choiceCount(const std::array<EnumChoice<Enum>, Size> &choices) {
        return static_cast<int>(choices.size());
    }

    template<typename Enum, size_t Size>
    constexpr int indexForValue(const Enum value, const std::array<EnumChoice<Enum>, Size> &choices) {
        for (size_t index = 0; index < choices.size(); ++index) {
            if (choices[index].value == value)
                return static_cast<int>(index);
        }

        return 0;
    }

    template<typename Enum, size_t Size>
    constexpr Enum valueForIndex(const int index, const std::array<EnumChoice<Enum>, Size> &choices) {
        const auto clampedIndex = juce::jlimit(0, static_cast<int>(choices.size()) - 1, index);
        return choices[static_cast<size_t>(clampedIndex)].value;
    }

    inline auto holdMsRange() {
        return juce::NormalisableRange<float>(Defaults::holdMsMin, Defaults::holdMsMax, Defaults::holdMsStep);
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

    inline juce::String slotParameterId(const SlotField field, const size_t slotIndex) {
        const auto slot = juce::String(static_cast<int>(slotIndex));

        switch (field) {
            case SlotField::enabled:
                return "signalSlot" + slot + "Enabled";
            case SlotField::visible:
                return "signalSlot" + slot + "Visible";
            case SlotField::frozen:
                return "signalSlot" + slot + "Frozen";
            case SlotField::source:
                return "signalSlot" + slot + "Source";
            case SlotField::mode:
                return "signalSlot" + slot + "Mode";
            case SlotField::colour:
                return "signalSlot" + slot + "Colour";
            case SlotField::opacity:
                return "signalSlot" + slot + "Opacity";
        }

        return {};
    }

    inline juce::String slotParameterName(const SlotField field, const size_t slotIndex) {
        const auto prefix = "Signal " + juce::String(static_cast<int>(slotIndex + 1)) + " ";

        switch (field) {
            case SlotField::enabled:
                return prefix + "Enabled";
            case SlotField::visible:
                return prefix + "Visible";
            case SlotField::frozen:
                return prefix + "Frozen";
            case SlotField::source:
                return prefix + "Source";
            case SlotField::mode:
                return prefix + "Mode";
            case SlotField::colour:
                return prefix + "Colour";
            case SlotField::opacity:
                return prefix + "Opacity";
        }

        return {};
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeSlotParameter(const SlotField field, const size_t slotIndex) {
        switch (field) {
            case SlotField::enabled:
                return std::make_unique<juce::AudioParameterBool>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    Defaults::isSignalSlotEnabled(slotIndex));
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
                    indexForValue(Defaults::signalSlotSource(slotIndex), signalSourceChoices));
            case SlotField::mode:
                return std::make_unique<juce::AudioParameterChoice>(
                    makeParameterID(slotParameterId(field, slotIndex)),
                    slotParameterName(field, slotIndex),
                    toStringArray(signalModeChoices),
                    indexForValue(Defaults::signalSlotMode(slotIndex), signalModeChoices));
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
            toStringArray(bandModeChoices),
            indexForValue(Defaults::bandMode, bandModeChoices)));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            makeParameterID(freezeId),
            freezeName,
            Defaults::freeze));

        constexpr std::array slotFields{
            SlotField::enabled,
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

        return layout;
    }
}
