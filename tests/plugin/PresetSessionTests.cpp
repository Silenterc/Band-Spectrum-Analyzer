#include <array>
#include <memory>

#include <catch2/catch_test_macros.hpp>

#include "plugin/parameters/ParameterAccess.h"
#include "plugin/parameters/ParameterSchema.h"
#include "plugin/presets/FactoryPresetRepository.h"
#include "plugin/presets/PluginStateSerializer.h"
#include "plugin/presets/PresetSession.h"
#include "plugin/presets/UserPresetStore.h"
#include "plugin/state/SignalSlotOrderState.h"

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

    struct TestPresetContext {
        TestAudioProcessor processor;
        juce::AudioProcessorValueTreeState parameters;
        PluginParameters::Access parameterAccess;
        SignalSlotOrderState signalSlotOrderState;
        PluginStateSerializer serializer;
        juce::File tempDirectory;
        UserPresetStore userPresetStore;
        FactoryPresetRepository factoryRepository;
        PresetSession presetSession;

        TestPresetContext()
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
                            userPresetStore) {
            parameterAccess.cache();
        }

        ~TestPresetContext() {
            tempDirectory.deleteRecursively();
        }

        PluginPresets::PresetDocument makeFactoryDefaultDocument() {
            PluginPresets::PresetDocument document;
            document.id = "factory-default";
            document.name = "Default";
            document.origin = PluginPresets::PresetOrigin::factory;
            document.createdAtUtc = "2026-04-18T00:00:00Z";
            document.updatedAtUtc = document.createdAtUtc;
            document.pluginState = serializer.captureState(parameters, signalSlotOrderState);
            return document;
        }

        Ui::Presets::PresetActionResult saveCurrentStateAs(const juce::String& name) {
            return presetSession.savePresetAs(name);
        }

        Ui::Presets::PresetActionResult deletePreset(const Ui::Presets::PresetId& presetId) {
            return presetSession.deletePreset(presetId);
        }
    };
}

TEST_CASE("PluginStateSerializer applies captured state while preserving UI scale", "[preset][serializer]") {
    TestPresetContext sourceContext;
    sourceContext.parameterAccess.writeFreeze(true);
    sourceContext.signalSlotOrderState.setOrder({ 2, 0, 1, 3 });

    if (auto* uiScaleParameter = sourceContext.parameters.getParameter(PluginParameters::Schema::uiScaleId))
        uiScaleParameter->setValueNotifyingHost(1.0f);

    const auto snapshot = sourceContext.serializer.captureState(sourceContext.parameters, sourceContext.signalSlotOrderState);

    TestPresetContext targetContext;
    if (auto* uiScaleParameter = targetContext.parameters.getParameter(PluginParameters::Schema::uiScaleId))
        uiScaleParameter->setValueNotifyingHost(0.0f);

    targetContext.parameterAccess.writeFreeze(false);
    targetContext.signalSlotOrderState.setOrder({ 0, 1, 2, 3 });

    REQUIRE(targetContext.serializer.applyState(snapshot, targetContext.parameters, targetContext.signalSlotOrderState));
    REQUIRE(targetContext.parameterAccess.readFreeze());
    REQUIRE(targetContext.signalSlotOrderState.getOrder() == Shared::SignalSlotOrder{ 2, 0, 1, 3 });
    REQUIRE(targetContext.parameterAccess.readUiScalePreset() == Ui::UiScalePreset::x1);
}

TEST_CASE("PluginStateSerializer treats migrated snapshots as equal after merge", "[preset][serializer]") {
    TestPresetContext context;

    const auto currentSnapshot = context.serializer.captureState(context.parameters, context.signalSlotOrderState);
    auto migratedSnapshot = currentSnapshot;
    migratedSnapshot.state.removeProperty(juce::Identifier(PluginParameters::Schema::freezeId), nullptr);

    for (int childIndex = migratedSnapshot.state.getNumChildren(); --childIndex >= 0;) {
        const auto child = migratedSnapshot.state.getChild(childIndex);
        if (child.hasProperty("id")
            && child.getProperty("id").toString() == PluginParameters::Schema::freezeId) {
            migratedSnapshot.state.removeChild(childIndex, nullptr);
            break;
        }
    }

    REQUIRE(context.serializer.statesEqual(currentSnapshot, migratedSnapshot));
    REQUIRE(context.serializer.statesEqual(migratedSnapshot, currentSnapshot));
}

TEST_CASE("UserPresetStore saves, overwrites, lists, and deletes user presets", "[preset][store]") {
    TestPresetContext context;

    PluginPresets::PresetDocument document;
    document.id = "user-1";
    document.name = "My Preset";
    document.origin = PluginPresets::PresetOrigin::user;
    document.createdAtUtc = "2026-04-18T00:00:00Z";
    document.updatedAtUtc = document.createdAtUtc;
    document.pluginState = context.serializer.captureState(context.parameters, context.signalSlotOrderState);

    REQUIRE(context.userPresetStore.save(document));
    REQUIRE(context.userPresetStore.loadAll().size() == 1);
    REQUIRE(context.tempDirectory.findChildFiles(juce::File::findFiles, false, "*.xml").size() == 1);

    document.name = "Renamed Preset";
    document.updatedAtUtc = "2026-04-18T00:10:00Z";
    REQUIRE(context.userPresetStore.save(document));

    const auto loadedDocuments = context.userPresetStore.loadAll();
    REQUIRE(loadedDocuments.size() == 1);
    REQUIRE(loadedDocuments.front().name == "Renamed Preset");
    REQUIRE(context.tempDirectory.findChildFiles(juce::File::findFiles, false, "*.xml").size() == 1);

    REQUIRE(context.userPresetStore.remove(document.id));
    REQUIRE(context.userPresetStore.loadAll().empty());
}

TEST_CASE("PresetSession saves, navigates, deletes, and reports dirty state", "[preset][session]") {
    TestPresetContext context;

    auto snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.selectedPresetName == "Default");
    REQUIRE(snapshot.selectionStatus == Ui::Presets::PresetSelectionStatus::selectedClean);

    context.parameterAccess.writeFreeze(true);
    context.presetSession.markCurrentStateDirty();
    snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.selectionStatus == Ui::Presets::PresetSelectionStatus::selectedDirty);

    REQUIRE(context.saveCurrentStateAs("Zulu").succeeded);
    REQUIRE(context.saveCurrentStateAs("Alpha").succeeded);

    snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.presets.size() == 3);
    REQUIRE(snapshot.presets[0].name == "Default");
    REQUIRE(snapshot.presets[1].name == "Alpha");
    REQUIRE(snapshot.presets[2].name == "Zulu");
    REQUIRE(snapshot.selectedPresetName == "Alpha");
    REQUIRE(snapshot.isSelectedPresetUser);
    REQUIRE(snapshot.selectionStatus == Ui::Presets::PresetSelectionStatus::selectedClean);

    REQUIRE(context.presetSession.loadPreviousPreset().succeeded);
    REQUIRE(context.presetSession.getSnapshot().selectedPresetName == "Default");
    REQUIRE_FALSE(context.parameterAccess.readFreeze());

    REQUIRE(context.presetSession.loadNextPreset().succeeded);
    REQUIRE(context.presetSession.getSnapshot().selectedPresetName == "Alpha");
    REQUIRE(context.parameterAccess.readFreeze());

    context.parameterAccess.writeFreeze(false);
    context.presetSession.markCurrentStateDirty();
    REQUIRE(context.presetSession.overwritePreset(*context.presetSession.getSelectedPresetId(), "Alpha").succeeded);
    REQUIRE_FALSE(context.parameterAccess.readFreeze());
    REQUIRE(context.presetSession.getSnapshot().selectionStatus == Ui::Presets::PresetSelectionStatus::selectedClean);

    const auto currentPresetId = context.presetSession.getSnapshot().selectedPresetId;
    REQUIRE(currentPresetId.has_value());
    REQUIRE(context.deletePreset(*currentPresetId).succeeded);

    snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.selectedPresetId.has_value());
    REQUIRE(snapshot.selectedPresetName == "Default");
    REQUIRE(snapshot.selectionStatus == Ui::Presets::PresetSelectionStatus::selectedClean);

    REQUIRE(context.presetSession.resetCurrentPreset().succeeded);
    snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.selectedPresetName == "Default");
    REQUIRE(snapshot.selectionStatus == Ui::Presets::PresetSelectionStatus::selectedClean);
}

TEST_CASE("PresetSession overwrites factory presets through user shadow overrides", "[preset][session]") {
    TestPresetContext context;

    context.parameterAccess.writeFreeze(true);
    context.presetSession.markCurrentStateDirty();
    REQUIRE(context.presetSession.overwritePreset("factory-default", "Default").succeeded);

    auto snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.presets.size() == 1);
    REQUIRE(snapshot.presets.front().id == "factory-default");
    REQUIRE(snapshot.presets.front().name == "Default");
    REQUIRE(snapshot.presets.front().origin == Ui::Presets::PresetOrigin::user);
    REQUIRE(snapshot.presets.front().shadowsFactoryPreset);
    REQUIRE(snapshot.presets.front().isDeletable);

    context.parameterAccess.writeFreeze(false);
    REQUIRE(context.presetSession.loadPreset("factory-default").succeeded);
    REQUIRE(context.parameterAccess.readFreeze());

    REQUIRE(context.deletePreset("factory-default").succeeded);
    snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.presets.size() == 1);
    REQUIRE(snapshot.presets.front().origin == Ui::Presets::PresetOrigin::factory);
    REQUIRE_FALSE(snapshot.presets.front().isDeletable);

    REQUIRE(context.presetSession.resetCurrentPreset().succeeded);
    REQUIRE_FALSE(context.parameterAccess.readFreeze());
}

TEST_CASE("PresetSession returns stable snapshots across repeated reads", "[preset][session]") {
    TestPresetContext context;

    const auto initialSnapshot = context.presetSession.getSnapshot();
    REQUIRE(context.presetSession.getSnapshot() == initialSnapshot);

    context.parameterAccess.writeFreeze(true);
    context.presetSession.markCurrentStateDirty();
    REQUIRE(context.saveCurrentStateAs("Alpha").succeeded);

    const auto afterSaveSnapshot = context.presetSession.getSnapshot();
    REQUIRE(context.presetSession.getSnapshot() == afterSaveSnapshot);

    const auto savedPresetId = afterSaveSnapshot.selectedPresetId;
    REQUIRE(savedPresetId.has_value());
    REQUIRE(context.deletePreset(*savedPresetId).succeeded);

    const auto afterDeleteSnapshot = context.presetSession.getSnapshot();
    REQUIRE(context.presetSession.getSnapshot() == afterDeleteSnapshot);
}

TEST_CASE("PresetSession refreshCatalog picks up external preset store changes", "[preset][session]") {
    TestPresetContext context;

    const auto initialSnapshot = context.presetSession.getSnapshot();
    REQUIRE(initialSnapshot.presets.size() == 1);

    PluginPresets::PresetDocument externalDocument;
    externalDocument.id = "external-user";
    externalDocument.name = "External";
    externalDocument.origin = PluginPresets::PresetOrigin::user;
    externalDocument.createdAtUtc = "2026-04-21T00:00:00Z";
    externalDocument.updatedAtUtc = externalDocument.createdAtUtc;
    externalDocument.pluginState = context.serializer.captureState(context.parameters, context.signalSlotOrderState);
    REQUIRE(context.userPresetStore.save(externalDocument));

    REQUIRE(context.presetSession.getSnapshot() == initialSnapshot);

    context.presetSession.refreshCatalog();
    const auto refreshedSnapshot = context.presetSession.getSnapshot();
    REQUIRE(refreshedSnapshot.presets.size() == 2);
    REQUIRE(refreshedSnapshot.presets[1].id == "external-user");
    REQUIRE(refreshedSnapshot.presets[1].name == "External");
}

TEST_CASE("PresetSession restores explicit preset selection when multiple presets share the same state",
          "[preset][session]") {
    TestPresetContext context;

    context.parameterAccess.writeFreeze(true);
    context.presetSession.markCurrentStateDirty();
    REQUIRE(context.saveCurrentStateAs("Alpha").succeeded);
    REQUIRE(context.saveCurrentStateAs("Bravo").succeeded);

    const auto snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.presets.size() == 3);
    const auto alphaId = snapshot.presets[1].id;
    const auto bravoId = snapshot.presets[2].id;

    context.presetSession.restoreSelection(alphaId);
    auto restoredSnapshot = context.presetSession.getSnapshot();
    REQUIRE(restoredSnapshot.selectedPresetId == std::optional<Ui::Presets::PresetId>(alphaId));
    REQUIRE(restoredSnapshot.selectedPresetName == "Alpha");

    context.presetSession.restoreSelection(bravoId);
    restoredSnapshot = context.presetSession.getSnapshot();
    REQUIRE(restoredSnapshot.selectedPresetId == std::optional<Ui::Presets::PresetId>(bravoId));
    REQUIRE(restoredSnapshot.selectedPresetName == "Bravo");
}

TEST_CASE("PresetSession keeps the selected preset during refresh when dirty state matches another preset",
          "[preset][session]") {
    TestPresetContext context;

    REQUIRE(context.saveCurrentStateAs("Alpha").succeeded);

    context.parameterAccess.writeFreeze(true);
    context.presetSession.markCurrentStateDirty();
    REQUIRE(context.saveCurrentStateAs("Beta").succeeded);

    const auto betaPresetId = context.presetSession.getSnapshot().selectedPresetId;
    REQUIRE(betaPresetId.has_value());
    REQUIRE(context.presetSession.getSnapshot().selectedPresetName == "Beta");

    context.parameterAccess.writeFreeze(false);
    context.presetSession.markCurrentStateDirty();
    context.presetSession.refreshCatalog();

    const auto snapshot = context.presetSession.getSnapshot();
    REQUIRE(snapshot.selectedPresetId == betaPresetId);
    REQUIRE(snapshot.selectedPresetName == "Beta");
    REQUIRE(snapshot.selectionStatus == Ui::Presets::PresetSelectionStatus::selectedDirty);
}

TEST_CASE("PresetSession save and delete actions return validation errors without mutating selection state",
          "[preset][session]") {
    TestPresetContext context;

    context.parameterAccess.writeFreeze(true);
    context.presetSession.markCurrentStateDirty();
    REQUIRE(context.saveCurrentStateAs("Alpha").succeeded);

    const auto selectedPresetId = context.presetSession.getSnapshot().selectedPresetId;
    REQUIRE(selectedPresetId.has_value());
    REQUIRE_FALSE(context.presetSession.savePresetAs("").succeeded);
    REQUIRE_FALSE(context.presetSession.overwritePreset(*selectedPresetId, "").succeeded);
    REQUIRE_FALSE(context.presetSession.deletePreset("missing-preset").succeeded);
    REQUIRE(context.presetSession.getSnapshot().selectedPresetId == selectedPresetId);
}
