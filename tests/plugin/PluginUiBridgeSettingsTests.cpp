#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "plugin/PluginUiBridge.h"
#include "plugin/parameters/ParameterAccess.h"
#include "plugin/parameters/ParameterSchema.h"
#include "plugin/presets/FactoryPresetRepository.h"
#include "plugin/presets/PluginStateSerializer.h"
#include "plugin/presets/PresetSession.h"
#include "plugin/presets/UserPresetStore.h"
#include "plugin/state/SignalSlotOrderState.h"
#include "shared/DefaultParameterValues.h"

namespace {
    class TestAudioProcessor final : public juce::AudioProcessor {
    public:
        TestAudioProcessor()
            : juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
        }

        void prepareToPlay(double, int) override {}
        void releaseResources() override {}
        bool isBusesLayoutSupported(const BusesLayout&) const override { return true; }
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
        using AudioProcessor::processBlock;
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        const juce::String getName() const override { return "TestProcessor"; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}
        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}
    };

    struct TestBridgeContext {
        juce::ScopedJuceInitialiser_GUI juceInitialiser;
        TestAudioProcessor processor;
        juce::AudioProcessorValueTreeState parameters;
        PluginParameters::Access parameterAccess;
        SignalSlotOrderState signalSlotOrderState;
        PluginStateSerializer serializer;
        juce::File tempDirectory;
        UserPresetStore userPresetStore;
        FactoryPresetRepository factoryRepository;
        PresetSession presetSession;
        PluginUiBridge bridge;

        TestBridgeContext()
            : parameters(processor, nullptr, "SpecParams", PluginParameters::Schema::makeParameterLayout()),
              parameterAccess(parameters),
              tempDirectory(juce::File::getSpecialLocation(juce::File::tempDirectory)
                                .getChildFile("band-spectrum-analyzer-tests")
                                .getChildFile(juce::Uuid().toString())),
              userPresetStore(tempDirectory),
              factoryRepository(std::vector<PluginPresets::PresetDocument>{makeFactoryDefaultDocument()}),
              presetSession(parameters,
                            signalSlotOrderState,
                            serializer,
                            factoryRepository,
                            userPresetStore),
              bridge(parameterAccess, signalSlotOrderState, presetSession, [] { return false; }) {
            parameterAccess.cache();
        }

        ~TestBridgeContext() {
            tempDirectory.deleteRecursively();
        }

        PluginPresets::PresetDocument makeFactoryDefaultDocument() {
            PluginPresets::PresetDocument document;
            document.id = "factory-default";
            document.name = "Default";
            document.origin = PluginPresets::PresetOrigin::factory;
            document.createdAtUtc = "2026-07-08T00:00:00Z";
            document.updatedAtUtc = document.createdAtUtc;
            document.pluginState = serializer.captureState(parameters, signalSlotOrderState);
            return document;
        }
    };
}

TEST_CASE("Grid min intent keeps the minimum span below grid max", "[bridge][settings]") {
    TestBridgeContext context;

    context.bridge.setGridMinDb(-60.0f);
    REQUIRE(context.parameterAccess.readGridMinDb() == Catch::Approx(-60.0f));

    // With grid max at -10 dB the relational bound (-16) is tighter than the -12 schema cap
    context.bridge.setGridMaxDb(-10.0f);
    context.bridge.setGridMinDb(0.0f);
    REQUIRE(context.parameterAccess.readGridMinDb() == Catch::Approx(-10.0f - Defaults::gridMinSpanDb));
}

TEST_CASE("Grid max intent keeps the minimum span above grid min", "[bridge][settings]") {
    TestBridgeContext context;

    context.bridge.setGridMinDb(-12.0f);
    context.bridge.setGridMaxDb(-20.0f);
    REQUIRE(context.parameterAccess.readGridMaxDb() == Catch::Approx(-12.0f + Defaults::gridMinSpanDb));
}

TEST_CASE("Visible min frequency intent stays at least an octave below max", "[bridge][settings]") {
    TestBridgeContext context;

    context.bridge.setVisibleMinFrequencyHz(15000.0f);
    REQUIRE(context.parameterAccess.readVisibleMinFrequencyHz()
            == Catch::Approx(Defaults::visibleMaxFrequencyHz / Defaults::visibleFrequencyMinSpanRatio));
}

TEST_CASE("Visible max frequency intent stays at least an octave above min", "[bridge][settings]") {
    TestBridgeContext context;

    context.bridge.setVisibleMinFrequencyHz(10000.0f);
    context.bridge.setVisibleMaxFrequencyHz(12000.0f);
    REQUIRE(context.parameterAccess.readVisibleMaxFrequencyHz()
            == Catch::Approx(10000.0f * Defaults::visibleFrequencyMinSpanRatio));
}

TEST_CASE("Band mode intent round-trips into the analyzer snapshot", "[bridge][settings]") {
    TestBridgeContext context;

    context.bridge.setBandMode(Analyzer::BandMode::octaveTwelfth);
    REQUIRE(context.parameterAccess.readBandMode() == Analyzer::BandMode::octaveTwelfth);
    REQUIRE(context.bridge.getAnalyzerUiSnapshot().bandMode == Analyzer::BandMode::octaveTwelfth);
}

TEST_CASE("Meter timing intents land in the snapshot and clamp to their ranges", "[bridge][settings]") {
    TestBridgeContext context;

    context.bridge.setHoldTimeMs(1500.0f);
    context.bridge.setRmsWindowMs(500.0f);
    context.bridge.setPeakDecayDbPerSecond(20.0f);
    context.bridge.setHoldDecayDbPerSecond(12.0f);

    const auto meterSettings = context.bridge.getAnalyzerUiSnapshot().meterSettings;
    REQUIRE(meterSettings.holdMs == Catch::Approx(1500.0f));
    REQUIRE(meterSettings.rmsWindowMs == Catch::Approx(500.0f));
    REQUIRE(meterSettings.peakDecayDbPerSecond == Catch::Approx(20.0f));
    REQUIRE(meterSettings.holdDecayDbPerSecond == Catch::Approx(12.0f));

    context.bridge.setHoldTimeMs(9000.0f);
    REQUIRE(context.bridge.getAnalyzerUiSnapshot().meterSettings.holdMs == Catch::Approx(Defaults::holdMsMax));
}

TEST_CASE("UI scale intent updates the editor presentation state", "[bridge][settings]") {
    TestBridgeContext context;

    context.bridge.setUiScalePreset(Ui::UiScalePreset::x2);
    REQUIRE(context.bridge.getEditorPresentationState().scale == Ui::UiScalePreset::x2);
}
