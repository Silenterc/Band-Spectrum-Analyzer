#pragma once

#include <array>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../dsp/BandMode.h"
#include "ParamIDs.h"
#include "../shared/DefaultParameterValues.h"
#include "../shared/SignalSlotConfiguration.h"
#include "../shared/SignalPresetCatalog.h"
#include "../ui/SignalSlotUiState.h"

namespace ParamSpec {
    inline constexpr int parameterVersionHint = 1;

    inline constexpr auto bandModeName = "Band Mode";
    inline constexpr auto freezeName = "Freeze";

    inline constexpr auto showRmsName = "Show RMS";
    inline constexpr auto showPeakName = "Show Peak";
    inline constexpr auto showHoldName = "Show Hold";

    inline constexpr auto holdMsName = "Hold Time";
    inline constexpr auto gridMinDbName = "Grid Min dB";
    inline constexpr auto gridMaxDbName = "Grid Max dB";
    inline constexpr auto gridStepDbName = "Grid Step dB";

    inline constexpr std::array<const char *, 3> bandModeChoices{
        "30 Bands",
        "45 Bands",
        "60 Bands"
    };

    inline constexpr std::array<const char *, 2> signalSourceChoices{
        "Main",
        "Sidechain"
    };

    inline constexpr std::array<const char *, 3> signalModeChoices{
        "Mid",
        "Side",
        "Stereo"
    };

    template<size_t size>
    juce::StringArray toStringArray(const std::array<const char *, size> &values) {
        juce::StringArray result;
        for (auto *value: values)
            result.add(value);
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

    inline auto holdMsRange() {
        return juce::NormalisableRange<float>(0.0f, 2000.0f, 1.0f);
    }

    inline auto gridMinDbRange() {
        return juce::NormalisableRange<float>(-120.0f, -12.0f, 1.0f);
    }

    inline auto gridMaxDbRange() {
        return juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f);
    }

    inline auto gridStepDbRange() {
        return juce::NormalisableRange<float>(1.0f, 24.0f, 1.0f);
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeBandModeParameter() {
        return std::make_unique<juce::AudioParameterChoice>(
            makeParameterID(ParamIDs::bandMode),
            bandModeName,
            toStringArray(bandModeChoices),
            static_cast<int>(Defaults::bandMode)
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeFreezeParameter() {
        return std::make_unique<juce::AudioParameterBool>(
            makeParameterID(ParamIDs::freeze),
            freezeName,
            Defaults::freeze
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeSignalSlotEnabledParameter(const int slotIndex,
                                                                                       const bool defaultValue) {
        return std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(ParamIDs::signalSlotEnabled(slotIndex), parameterVersionHint),
            "Signal " + juce::String(slotIndex + 1) + " Enabled",
            defaultValue
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeSignalSlotVisibleParameter(const int slotIndex,
                                                                                       const bool defaultValue) {
        return std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(ParamIDs::signalSlotVisible(slotIndex), parameterVersionHint),
            "Signal " + juce::String(slotIndex + 1) + " Visible",
            defaultValue
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeSignalSlotSourceParameter(const int slotIndex,
                                                                                      const Analyzer::SignalSource defaultValue) {
        return std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(ParamIDs::signalSlotSource(slotIndex), parameterVersionHint),
            "Signal " + juce::String(slotIndex + 1) + " Source",
            toStringArray(signalSourceChoices),
            static_cast<int>(defaultValue)
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeSignalSlotModeParameter(const int slotIndex,
                                                                                    const Analyzer::SignalMode defaultValue) {
        return std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(ParamIDs::signalSlotMode(slotIndex), parameterVersionHint),
            "Signal " + juce::String(slotIndex + 1) + " Mode",
            toStringArray(signalModeChoices),
            static_cast<int>(defaultValue)
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeSignalSlotColourParameter(const int slotIndex,
                                                                                      const int defaultValue) {
        return std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(ParamIDs::signalSlotColour(slotIndex), parameterVersionHint),
            "Signal " + juce::String(slotIndex + 1) + " Colour",
            makeSignalColourChoiceArray(),
            defaultValue
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeSignalSlotOpacityParameter(const int slotIndex,
                                                                                       const float defaultValue) {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::signalSlotOpacity(slotIndex), parameterVersionHint),
            "Signal " + juce::String(slotIndex + 1) + " Opacity",
            juce::NormalisableRange<float>(0.15f, 1.0f, 0.01f),
            defaultValue
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeShowRmsParameter() {
        return std::make_unique<juce::AudioParameterBool>(
            makeParameterID(ParamIDs::showRms),
            showRmsName,
            Defaults::showRms
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeShowPeakParameter() {
        return std::make_unique<juce::AudioParameterBool>(
            makeParameterID(ParamIDs::showPeak),
            showPeakName,
            Defaults::showPeak
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeShowHoldParameter() {
        return std::make_unique<juce::AudioParameterBool>(
            makeParameterID(ParamIDs::showHold),
            showHoldName,
            Defaults::showHold
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeHoldMsParameter() {
        return std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(ParamIDs::holdMs),
            holdMsName,
            holdMsRange(),
            Defaults::holdMs
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeGridMinDbParameter() {
        return std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(ParamIDs::gridMinDb),
            gridMinDbName,
            gridMinDbRange(),
            Defaults::gridMinDb
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeGridMaxDbParameter() {
        return std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(ParamIDs::gridMaxDb),
            gridMaxDbName,
            gridMaxDbRange(),
            Defaults::gridMaxDb
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeGridStepDbParameter() {
        return std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(ParamIDs::gridStepDb),
            gridStepDbName,
            gridStepDbRange(),
            Defaults::gridStepDb
        );
    }
}
