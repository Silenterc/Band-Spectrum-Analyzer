#pragma once

#include <array>

#include <juce_audio_processors/juce_audio_processors.h>

#include "ParamIDs.h"

namespace ParamSpec {
    inline constexpr int parameterVersionHint = 1;

    enum class AnalysisMode {
        summed = 0,
        midSide,
        stereo
    };

    enum class BandMode {
        bands30 = 0,
        bands45,
        bands60
    };

    inline constexpr auto analysisModeName = "Analysis Mode";
    inline constexpr auto bandModeName = "Band Mode";

    inline constexpr auto showRmsName = "Show RMS";
    inline constexpr auto showPeakName = "Show Peak";
    inline constexpr auto showHoldName = "Show Hold";

    inline constexpr auto holdMsName = "Hold Time";
    inline constexpr auto gridMinDbName = "Grid Min dB";
    inline constexpr auto gridMaxDbName = "Grid Max dB";
    inline constexpr auto gridStepDbName = "Grid Step dB";

    inline constexpr std::array<const char *, 3> analysisModeChoices{
        "Summed",
        "Mid/Side",
        "Stereo"
    };

    inline constexpr std::array<const char *, 3> bandModeChoices{
        "30 Bands",
        "45 Bands",
        "60 Bands"
    };

    template<size_t size>
    juce::StringArray toStringArray(const std::array<const char *, size> &values) {
        juce::StringArray result;
        for (auto *value: values)
            result.add(value);
        return result;
    }

    inline juce::ParameterID makeParameterID(const char *id) {
        return {id, parameterVersionHint};
    }

    inline constexpr int defaultAnalysisMode = static_cast<int>(AnalysisMode::stereo);
    inline constexpr int defaultBandMode = static_cast<int>(BandMode::bands45);

    inline constexpr bool defaultShowRms = true;
    inline constexpr bool defaultShowPeak = true;
    inline constexpr bool defaultShowHold = true;

    inline constexpr float defaultHoldMs = 500.0f;
    inline constexpr float defaultGridMinDb = -50.0f;
    inline constexpr float defaultGridMaxDb = 0.0f;
    inline constexpr float defaultGridStepDb = 5.0f;

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

    inline std::unique_ptr<juce::RangedAudioParameter> makeAnalysisModeParameter() {
        return std::make_unique<juce::AudioParameterChoice>(
            makeParameterID(ParamIDs::analysisMode),
            analysisModeName,
            toStringArray(analysisModeChoices),
            defaultAnalysisMode
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeBandModeParameter() {
        return std::make_unique<juce::AudioParameterChoice>(
            makeParameterID(ParamIDs::bandMode),
            bandModeName,
            toStringArray(bandModeChoices),
            defaultBandMode
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeShowRmsParameter() {
        return std::make_unique<juce::AudioParameterBool>(
            makeParameterID(ParamIDs::showRms),
            showRmsName,
            defaultShowRms
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeShowPeakParameter() {
        return std::make_unique<juce::AudioParameterBool>(
            makeParameterID(ParamIDs::showPeak),
            showPeakName,
            defaultShowPeak
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeShowHoldParameter() {
        return std::make_unique<juce::AudioParameterBool>(
            makeParameterID(ParamIDs::showHold),
            showHoldName,
            defaultShowHold
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeHoldMsParameter() {
        return std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(ParamIDs::holdMs),
            holdMsName,
            holdMsRange(),
            defaultHoldMs
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeGridMinDbParameter() {
        return std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(ParamIDs::gridMinDb),
            gridMinDbName,
            gridMinDbRange(),
            defaultGridMinDb
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeGridMaxDbParameter() {
        return std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(ParamIDs::gridMaxDb),
            gridMaxDbName,
            gridMaxDbRange(),
            defaultGridMaxDb
        );
    }

    inline std::unique_ptr<juce::RangedAudioParameter> makeGridStepDbParameter() {
        return std::make_unique<juce::AudioParameterFloat>(
            makeParameterID(ParamIDs::gridStepDb),
            gridStepDbName,
            gridStepDbRange(),
            defaultGridStepDb
        );
    }
}
