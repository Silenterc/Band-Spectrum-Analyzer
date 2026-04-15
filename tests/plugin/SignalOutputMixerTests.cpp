#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "plugin/SignalOutputMixer.h"
#include "plugin/parameters/ParameterAccess.h"
#include "plugin/parameters/ParameterSchema.h"

namespace {
    class TestAudioProcessor final : public juce::AudioProcessor {
    public:
        TestAudioProcessor()
            : juce::AudioProcessor(BusesProperties()
                                       .withInput("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
        }

        const juce::String getName() const override { return "TestProcessor"; }
        void prepareToPlay(double sampleRate, int samplesPerBlock) override { juce::ignoreUnused(sampleRate, samplesPerBlock); }
        void releaseResources() override {}
        bool isBusesLayoutSupported(const BusesLayout &layouts) const override {
            return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
        }

        void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override {}
        juce::AudioProcessorEditor *createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int index) override { juce::ignoreUnused(index); }
        const juce::String getProgramName(int index) override {
            juce::ignoreUnused(index);
            return {};
        }
        void changeProgramName(int index, const juce::String &newName) override {
            juce::ignoreUnused(index, newName);
        }
        void getStateInformation(juce::MemoryBlock &destData) override { destData.reset(); }
        void setStateInformation(const void *data, int sizeInBytes) override {
            juce::ignoreUnused(data, sizeInBytes);
        }
    };

    struct ParameterAccessFixture {
        TestAudioProcessor processor;
        juce::AudioProcessorValueTreeState parameters {
            processor,
            nullptr,
            "TestState",
            PluginParameters::Schema::makeParameterLayout()
        };
        PluginParameters::Access access {parameters};

        ParameterAccessFixture() {
            access.cache();
        }
    };

    juce::AudioBuffer<float> makeStereoBuffer(const std::initializer_list<float> leftSamples,
                                              const std::initializer_list<float> rightSamples) {
        const auto numSamples = static_cast<int>(leftSamples.size());
        REQUIRE(rightSamples.size() == leftSamples.size());

        juce::AudioBuffer<float> buffer(2, numSamples);
        int sampleIndex = 0;
        for (const auto sample: leftSamples)
            buffer.setSample(0, sampleIndex++, sample);

        sampleIndex = 0;
        for (const auto sample: rightSamples)
            buffer.setSample(1, sampleIndex++, sample);

        return buffer;
    }

    std::array<SignalOutputMixer::SlotState, Shared::maxSignalSlots> makeSlots() {
        std::array<SignalOutputMixer::SlotState, Shared::maxSignalSlots> slots{};
        for (auto &slot: slots) {
            slot.source = Analyzer::SignalSource::main;
            slot.mode = Analyzer::SignalMode::mid;
        }
        return slots;
    }

    void requireChannelEquals(const juce::AudioBuffer<float> &buffer,
                              const int channelIndex,
                              const std::initializer_list<float> expectedSamples) {
        REQUIRE(buffer.getNumSamples() == static_cast<int>(expectedSamples.size()));

        int sampleIndex = 0;
        for (const auto expected: expectedSamples) {
            REQUIRE(buffer.getSample(channelIndex, sampleIndex)
                    == Catch::Approx(expected).margin(1.0e-6));
            ++sampleIndex;
        }
    }

    void requireBufferEquals(const juce::AudioBuffer<float> &buffer,
                             const juce::AudioBuffer<float> &expectedBuffer) {
        REQUIRE(buffer.getNumChannels() == expectedBuffer.getNumChannels());
        REQUIRE(buffer.getNumSamples() == expectedBuffer.getNumSamples());

        for (int channelIndex = 0; channelIndex < buffer.getNumChannels(); ++channelIndex) {
            for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex) {
                REQUIRE(buffer.getSample(channelIndex, sampleIndex)
                        == Catch::Approx(expectedBuffer.getSample(channelIndex, sampleIndex)).margin(1.0e-6));
            }
        }
    }
}

TEST_CASE("SignalOutputMixer leaves output untouched when no solo is active", "[plugin][solo]") {
    SignalOutputMixer mixer;
    mixer.prepare(8);

    auto mainBuffer = makeStereoBuffer({1.0f, 2.0f}, {3.0f, 4.0f});
    const auto originalBuffer = mainBuffer;
    const auto slots = makeSlots();

    mixer.processBlock(mainBuffer, nullptr, slots);

    requireBufferEquals(mainBuffer, originalBuffer);
}

TEST_CASE("SignalOutputMixer outputs main stereo once when stereo is soloed", "[plugin][solo]") {
    SignalOutputMixer mixer;
    mixer.prepare(8);

    auto mainBuffer = makeStereoBuffer({1.0f, 2.0f}, {3.0f, 4.0f});
    auto slots = makeSlots();
    slots[0].enabled = true;
    slots[0].solo = true;
    slots[0].source = Analyzer::SignalSource::main;
    slots[0].mode = Analyzer::SignalMode::stereo;

    mixer.processBlock(mainBuffer, nullptr, slots);

    requireChannelEquals(mainBuffer, 0, {1.0f, 2.0f});
    requireChannelEquals(mainBuffer, 1, {3.0f, 4.0f});
}

TEST_CASE("SignalOutputMixer outputs main mid as dual mono when mid is soloed", "[plugin][solo]") {
    SignalOutputMixer mixer;
    mixer.prepare(8);

    auto mainBuffer = makeStereoBuffer({1.0f, 2.0f}, {3.0f, 4.0f});
    auto slots = makeSlots();
    slots[0].enabled = true;
    slots[0].solo = true;
    slots[0].mode = Analyzer::SignalMode::mid;

    mixer.processBlock(mainBuffer, nullptr, slots);

    requireChannelEquals(mainBuffer, 0, {2.0f, 3.0f});
    requireChannelEquals(mainBuffer, 1, {2.0f, 3.0f});
}

TEST_CASE("SignalOutputMixer outputs main side with opposite polarity per channel", "[plugin][solo]") {
    SignalOutputMixer mixer;
    mixer.prepare(8);

    auto mainBuffer = makeStereoBuffer({1.0f, 2.0f}, {3.0f, 4.0f});
    auto slots = makeSlots();
    slots[0].enabled = true;
    slots[0].solo = true;
    slots[0].mode = Analyzer::SignalMode::side;

    mixer.processBlock(mainBuffer, nullptr, slots);

    requireChannelEquals(mainBuffer, 0, {-1.0f, -1.0f});
    requireChannelEquals(mainBuffer, 1, {1.0f, 1.0f});
}

TEST_CASE("SignalOutputMixer reconstructs stereo from mid and side solos on the same source", "[plugin][solo]") {
    SignalOutputMixer mixer;
    mixer.prepare(8);

    auto mainBuffer = makeStereoBuffer({1.0f, 2.0f}, {3.0f, 4.0f});
    auto slots = makeSlots();
    slots[0].enabled = true;
    slots[0].solo = true;
    slots[0].mode = Analyzer::SignalMode::mid;
    slots[1].enabled = true;
    slots[1].solo = true;
    slots[1].mode = Analyzer::SignalMode::side;

    mixer.processBlock(mainBuffer, nullptr, slots);

    requireChannelEquals(mainBuffer, 0, {1.0f, 2.0f});
    requireChannelEquals(mainBuffer, 1, {3.0f, 4.0f});
}

TEST_CASE("SignalOutputMixer treats stereo as dominant within one source", "[plugin][solo]") {
    SignalOutputMixer mixer;
    mixer.prepare(8);

    auto mainBuffer = makeStereoBuffer({1.0f, 2.0f}, {3.0f, 4.0f});
    auto slots = makeSlots();
    slots[0].enabled = true;
    slots[0].solo = true;
    slots[0].mode = Analyzer::SignalMode::stereo;
    slots[1].enabled = true;
    slots[1].solo = true;
    slots[1].mode = Analyzer::SignalMode::mid;

    mixer.processBlock(mainBuffer, nullptr, slots);

    requireChannelEquals(mainBuffer, 0, {1.0f, 2.0f});
    requireChannelEquals(mainBuffer, 1, {3.0f, 4.0f});
}

TEST_CASE("SignalOutputMixer sums resolved main and sidechain sources without normalization", "[plugin][solo]") {
    SignalOutputMixer mixer;
    mixer.prepare(8);

    auto mainBuffer = makeStereoBuffer({1.0f, 2.0f}, {3.0f, 4.0f});
    const auto sidechainBuffer = makeStereoBuffer({10.0f, 14.0f}, {6.0f, 10.0f});
    auto slots = makeSlots();
    slots[0].enabled = true;
    slots[0].solo = true;
    slots[0].source = Analyzer::SignalSource::main;
    slots[0].mode = Analyzer::SignalMode::stereo;
    slots[1].enabled = true;
    slots[1].solo = true;
    slots[1].source = Analyzer::SignalSource::sidechain;
    slots[1].mode = Analyzer::SignalMode::mid;

    mixer.processBlock(mainBuffer, &sidechainBuffer, slots);

    requireChannelEquals(mainBuffer, 0, {9.0f, 14.0f});
    requireChannelEquals(mainBuffer, 1, {11.0f, 16.0f});
}

TEST_CASE("SignalOutputMixer ignores disabled soloed slots", "[plugin][solo]") {
    SignalOutputMixer mixer;
    mixer.prepare(8);

    auto mainBuffer = makeStereoBuffer({1.0f, 2.0f}, {3.0f, 4.0f});
    const auto originalBuffer = mainBuffer;
    auto slots = makeSlots();
    slots[0].enabled = false;
    slots[0].solo = true;
    slots[0].mode = Analyzer::SignalMode::stereo;

    mixer.processBlock(mainBuffer, nullptr, slots);

    requireBufferEquals(mainBuffer, originalBuffer);
}

TEST_CASE("ParameterAccess exposes solo in UI state but not engine state", "[plugin][solo][params]") {
    ParameterAccessFixture fixture;
    const auto engineStateBefore = fixture.access.readEngineState();

    fixture.access.writeSlotSolo(0, true);

    const auto uiState = fixture.access.readUiSlot(0);
    const auto engineStateAfter = fixture.access.readEngineState();

    REQUIRE(uiState.solo);
    REQUIRE(engineStateAfter == engineStateBefore);
}
