#pragma once

#include <array>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../../display/analyzer/data/AnalyzerMeterData.h"
#include "../../dsp/core/EngineParameterState.h"
#include "../../ui/state/SignalSlotUiState.h"
#include "ParameterSchema.h"

namespace PluginParameters {
    class Access final {
    public:
        explicit Access(juce::AudioProcessorValueTreeState &parametersToUse);

        void cache();

        Analyzer::EngineParameterState readEngineState() const;
        std::array<Ui::SignalSlotState, Shared::maxSignalSlots> readUiSlots() const;
        Ui::SignalSlotState readUiSlot(size_t slotIndex) const;
        Analyzer::MeterSettings readMeterSettings() const;
        Analyzer::BandMode readBandMode() const;
        bool readFreeze() const;
        float readGridMinDb() const;
        float readGridMaxDb() const;
        float readGridStepDb() const;
        bool readUseCustomFrequencyRange() const;
        float readVisibleMinFrequencyHz() const;
        float readVisibleMaxFrequencyHz() const;

        void writeFreeze(bool value);
        void writeSlotState(size_t slotIndex, const Ui::SignalSlotState &state);
        void writeSlotSignal(size_t slotIndex, Analyzer::SignalSource source, Analyzer::SignalMode mode);
        void writeSlotEnabled(size_t slotIndex, bool value);
        void writeSlotVisible(size_t slotIndex, bool value);
        void writeSlotFrozen(size_t slotIndex, bool value);
        void writeSlotColour(size_t slotIndex, int colourIndex);
        void writeSlotOpacity(size_t slotIndex, float opacity);
        void writeShowPeak(bool value);
        void writeShowRms(bool value);
        void writeShowHold(bool value);
        void writeUseCustomFrequencyRange(bool value);
        void writeVisibleMinFrequencyHz(float frequencyHz);
        void writeVisibleMaxFrequencyHz(float frequencyHz);

    private:
        struct SlotRefs {
            std::atomic<float> *enabled = nullptr;
            std::atomic<float> *visible = nullptr;
            std::atomic<float> *frozen = nullptr;
            std::atomic<float> *source = nullptr;
            std::atomic<float> *mode = nullptr;
            std::atomic<float> *colour = nullptr;
            std::atomic<float> *opacity = nullptr;
        };

        template<typename Enum, size_t Size>
        Enum readChoice(std::atomic<float> *parameter,
                        const std::array<Schema::EnumChoice<Enum>, Size> &choices) const {
            jassert(parameter != nullptr);
            const auto index = juce::roundToInt(parameter->load());
            return Schema::valueForIndex(index, choices);
        }

        static bool readBool(std::atomic<float> *parameter);
        static float readFloat(std::atomic<float> *parameter);

        void writeBool(const juce::String &parameterId, bool value);
        void writeChoiceIndex(const juce::String &parameterId, int index, int choiceCount);
        void writeFloat(const juce::String &parameterId, float plainValue);

        juce::AudioProcessorValueTreeState &parameters;
        std::atomic<float> *bandModeParam = nullptr;
        std::atomic<float> *freezeParam = nullptr;
        std::array<SlotRefs, Shared::maxSignalSlots> slotParams{};
        std::atomic<float> *showRmsParam = nullptr;
        std::atomic<float> *showPeakParam = nullptr;
        std::atomic<float> *showHoldParam = nullptr;
        std::atomic<float> *holdMsParam = nullptr;
        std::atomic<float> *rmsWindowMsParam = nullptr;
        std::atomic<float> *gridMinDbParam = nullptr;
        std::atomic<float> *gridMaxDbParam = nullptr;
        std::atomic<float> *gridStepDbParam = nullptr;
        std::atomic<float> *useCustomFrequencyRangeParam = nullptr;
        std::atomic<float> *visibleMinFrequencyHzParam = nullptr;
        std::atomic<float> *visibleMaxFrequencyHzParam = nullptr;
    };
}
