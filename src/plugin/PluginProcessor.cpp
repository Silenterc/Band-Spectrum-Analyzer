#include "PluginProcessor.h"
#include "../ui/editor/PluginEditor.h"

namespace {
    constexpr auto selectedPresetIdProperty = "__selectedPresetId";

    std::array<SignalOutputMixer::SlotState, Shared::maxSignalSlots> makeMixerSlotStates(
        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &uiSlots) {
        std::array<SignalOutputMixer::SlotState, Shared::maxSignalSlots> mixerSlots{};

        for (size_t slotIndex = 0; slotIndex < uiSlots.size(); ++slotIndex) {
            const auto &uiSlot = uiSlots[slotIndex];
            auto &mixerSlot = mixerSlots[slotIndex];
            mixerSlot.enabled = uiSlot.configuration.enabled;
            mixerSlot.solo = uiSlot.solo;
            mixerSlot.source = uiSlot.configuration.source;
            mixerSlot.mode = uiSlot.configuration.mode;
        }

        return mixerSlots;
    }
}

//==============================================================================
SpectrumAnalyzerAudioProcessor::SpectrumAnalyzerAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withInput("Sidechain", juce::AudioChannelSet::stereo(), false)
#endif
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
          // TODO: IMPLEMENT UNDO
    ),
      parameters(*this, nullptr, "SpecParams", createParameterLayout()),
      parameterAccess(parameters),
      userPresetStore(UserPresetStore::defaultPresetDirectory()),
      factoryPresetRepository(pluginStateSerializer.captureState(parameters, signalSlotOrderState)),
      presetSession(parameters,
                    signalSlotOrderState,
                    pluginStateSerializer,
                    factoryPresetRepository,
                    userPresetStore) {
    parameterAccess.cache();
    changeTracker = std::make_unique<ProcessorChangeTracker>(
        parameters,
        signalSlotOrderState,
        static_cast<ProcessorChangeTracker::Listener &>(*this));
}

const juce::String SpectrumAnalyzerAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool SpectrumAnalyzerAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SpectrumAnalyzerAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SpectrumAnalyzerAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SpectrumAnalyzerAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int SpectrumAnalyzerAudioProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SpectrumAnalyzerAudioProcessor::getCurrentProgram() {
    return 0;
}

void SpectrumAnalyzerAudioProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String SpectrumAnalyzerAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void SpectrumAnalyzerAudioProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    engine.prepare(sampleRate, samplesPerBlock);
    outputMixer.prepare(samplesPerBlock);
    previousEngineParameters = parameterAccess.readEngineState();
    engine.setParameters(*previousEngineParameters);
    changeTracker->clearEngineParametersDirty();
}

void SpectrumAnalyzerAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void SpectrumAnalyzerAudioProcessor::numBusesChanged() {
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::processorLayoutsChanged() {
    changeTracker->requestUiRefresh();
}

bool SpectrumAnalyzerAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    if (layouts.inputBuses.size() > 1) {
        const auto sidechainLayout = layouts.getChannelSet(true, 1);
        if (!sidechainLayout.isDisabled()
            && sidechainLayout != juce::AudioChannelSet::mono()
            && sidechainLayout != juce::AudioChannelSet::stereo())
            return false;
    }

    return true;
#endif
}

void SpectrumAnalyzerAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                                  juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);

    // Zeroes out really tiny floats (denormals)
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (changeTracker->consumeEngineParametersDirty()) {
        const auto currentEngineParameters = parameterAccess.readEngineState();

        if (!previousEngineParameters.has_value()
            || *previousEngineParameters != currentEngineParameters) {
            engine.setParameters(currentEngineParameters);
            previousEngineParameters = currentEngineParameters;
        }
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    auto mainBuffer = getBusBuffer(buffer, true, 0);
    const auto mixerSlotStates = makeMixerSlotStates(parameterAccess.readUiSlots());

    if (getBusCount(true) > 1 && getChannelCountOfBus(true, 1) > 0) {
        auto sidechainBuffer = getBusBuffer(buffer, true, 1);
        engine.processBlock(mainBuffer, &sidechainBuffer);
        outputMixer.processBlock(mainBuffer, &sidechainBuffer, mixerSlotStates);
        return;
    }

    engine.processBlock(mainBuffer);
    outputMixer.processBlock(mainBuffer, nullptr, mixerSlotStates);
}

//==============================================================================
bool SpectrumAnalyzerAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *SpectrumAnalyzerAudioProcessor::createEditor() {
    return new SpectrumAnalyzerAudioProcessorEditor(*this);
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    auto snapshot = pluginStateSerializer.captureState(parameters, signalSlotOrderState);
    if (const auto selectedPresetId = presetSession.getSelectedPresetId(); selectedPresetId.has_value())
        snapshot.state.setProperty(selectedPresetIdProperty, *selectedPresetId, nullptr);
    else
        snapshot.state.removeProperty(juce::Identifier(selectedPresetIdProperty), nullptr);

    if (const auto stateXml = snapshot.state.createXml())
        copyXmlToBinary(*stateXml, destData);
}

void SpectrumAnalyzerAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    const auto stateXml = getXmlFromBinary(data, sizeInBytes);
    if (stateXml == nullptr)
        return;

    PluginPresets::PluginStateSnapshot snapshot;
    snapshot.state = juce::ValueTree::fromXml(*stateXml);
    if (!snapshot.isValid())
        return;

    std::optional<PluginPresets::PresetId> restoredSelectedPresetId;
    if (snapshot.state.hasProperty(selectedPresetIdProperty)) {
        const auto presetId = snapshot.state.getProperty(selectedPresetIdProperty).toString();
        if (presetId.isNotEmpty())
            restoredSelectedPresetId = presetId;

        snapshot.state.removeProperty(juce::Identifier(selectedPresetIdProperty), nullptr);
    }

    if (!pluginStateSerializer.applyState(snapshot, parameters, signalSlotOrderState))
        return;

    presetSession.markCurrentStateDirty();
    parameterAccess.cache();
    previousEngineParameters = parameterAccess.readEngineState();
    if (previousEngineParameters.has_value())
        engine.setParameters(*previousEngineParameters);

    presetSession.restoreSelection(restoredSelectedPresetId);
    changeTracker->clearEngineParametersDirty();
    changeTracker->requestUiRefresh();
}

std::shared_ptr<const std::vector<Analyzer::BandInfo>> SpectrumAnalyzerAudioProcessor::getBandInfo() const {
    return engine.getBandInfo();
}

Ui::AnalyzerUiSnapshot SpectrumAnalyzerAudioProcessor::getAnalyzerUiSnapshot() const {
    Ui::AnalyzerUiSnapshot state;
    state.signalSlots = getSignalSlots();
    state.slotOrder = getSignalSlotOrder();
    state.meterSettings = getMeterSettings();
    state.frozen = isFrozen();
    state.sidechainAvailable = isSidechainAvailable();
    state.gridMinDb = getGridMinDb();
    state.gridMaxDb = getGridMaxDb();
    state.gridStepDb = getGridStepDb();
    state.useCustomFrequencyRange = getUseCustomFrequencyRange();
    state.visibleMinFrequencyHz = getVisibleMinFrequencyHz();
    state.visibleMaxFrequencyHz = getVisibleMaxFrequencyHz();
    return state;
}

void SpectrumAnalyzerAudioProcessor::addAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) {
    uiSnapshotListeners.add(&listener);
}

void SpectrumAnalyzerAudioProcessor::removeAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) {
    uiSnapshotListeners.remove(&listener);
}

Ui::EditorPresentationState SpectrumAnalyzerAudioProcessor::getEditorPresentationState() const {
    Ui::EditorPresentationState state;
    state.scale = getUiScalePreset();
    return state;
}

void SpectrumAnalyzerAudioProcessor::addEditorPresentationStateListener(EditorPresentationStateSource::Listener &listener) {
    editorPresentationStateListeners.add(&listener);
}

void SpectrumAnalyzerAudioProcessor::removeEditorPresentationStateListener(
    EditorPresentationStateSource::Listener &listener) {
    editorPresentationStateListeners.remove(&listener);
}

PluginPresets::PresetUiSnapshot SpectrumAnalyzerAudioProcessor::getPresetUiSnapshot() const {
    return presetSession.getSnapshot();
}

void SpectrumAnalyzerAudioProcessor::addPresetUiSnapshotListener(PresetUiSnapshotSource::Listener& listener) {
    presetUiSnapshotListeners.add(&listener);
}

void SpectrumAnalyzerAudioProcessor::removePresetUiSnapshotListener(PresetUiSnapshotSource::Listener& listener) {
    presetUiSnapshotListeners.remove(&listener);
}

AnalyzerPublishedTracesView SpectrumAnalyzerAudioProcessor::readPublishedTraces() const {
    return engine.readPublishedTraces();
}

std::array<Ui::SignalSlotState, Shared::maxSignalSlots> SpectrumAnalyzerAudioProcessor::getSignalSlots() const {
    return parameterAccess.readUiSlots();
}

Shared::SignalSlotOrder SpectrumAnalyzerAudioProcessor::getSignalSlotOrder() const {
    return signalSlotOrderState.getOrder();
}

bool SpectrumAnalyzerAudioProcessor::isSidechainAvailable() const {
    return getBusCount(true) > 1 && getChannelCountOfBus(true, 1) > 0;
}

bool SpectrumAnalyzerAudioProcessor::hasRecentSignal() const {
    return engine.hasRecentSignal();
}

bool SpectrumAnalyzerAudioProcessor::shouldProcessAnalyzer() const {
    return engine.shouldProcessAnalyzer();
}

bool SpectrumAnalyzerAudioProcessor::isFrozen() const {
    return parameterAccess.readFreeze();
}

Analyzer::MeterSettings SpectrumAnalyzerAudioProcessor::getMeterSettings() const {
    return parameterAccess.readMeterSettings();
}

float SpectrumAnalyzerAudioProcessor::getGridMinDb() const {
    return parameterAccess.readGridMinDb();
}

float SpectrumAnalyzerAudioProcessor::getGridMaxDb() const {
    return parameterAccess.readGridMaxDb();
}

float SpectrumAnalyzerAudioProcessor::getGridStepDb() const {
    return parameterAccess.readGridStepDb();
}

bool SpectrumAnalyzerAudioProcessor::getUseCustomFrequencyRange() const {
    return parameterAccess.readUseCustomFrequencyRange();
}

float SpectrumAnalyzerAudioProcessor::getVisibleMinFrequencyHz() const {
    return parameterAccess.readVisibleMinFrequencyHz();
}

float SpectrumAnalyzerAudioProcessor::getVisibleMaxFrequencyHz() const {
    return parameterAccess.readVisibleMaxFrequencyHz();
}

Ui::UiScalePreset SpectrumAnalyzerAudioProcessor::getUiScalePreset() const {
    return parameterAccess.readUiScalePreset();
}

juce::AudioProcessorValueTreeState::ParameterLayout SpectrumAnalyzerAudioProcessor::createParameterLayout() {
    return PluginParameters::Schema::makeParameterLayout();
}

void SpectrumAnalyzerAudioProcessor::setFreezeEnabled(const bool isFrozenValue) {
    parameterAccess.writeFreeze(isFrozenValue);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotEnabled(const size_t slotIndex, const bool isEnabled) {
    parameterAccess.writeSlotEnabled(slotIndex, isEnabled);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotVisible(const size_t slotIndex, const bool isVisible) {
    parameterAccess.writeSlotVisible(slotIndex, isVisible);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotFrozen(const size_t slotIndex, const bool isFrozenValue) {
    parameterAccess.writeSlotFrozen(slotIndex, isFrozenValue);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotSolo(const size_t slotIndex, const bool isSolo) {
    parameterAccess.writeSlotSolo(slotIndex, isSolo);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotSource(const size_t slotIndex, const Analyzer::SignalSource source) {
    parameterAccess.writeSlotSignal(slotIndex, source, parameterAccess.readUiSlot(slotIndex).configuration.mode);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotMode(const size_t slotIndex, const Analyzer::SignalMode mode) {
    parameterAccess.writeSlotSignal(slotIndex, parameterAccess.readUiSlot(slotIndex).configuration.source, mode);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::removeSignalSlot(const size_t slotIndex) {
    auto state = parameterAccess.readUiSlot(slotIndex);
    state.configuration.enabled = false;
    state.visible = false;
    state.frozen = false;
    parameterAccess.writeSlotState(slotIndex, state);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::addSignalSlot(const size_t slotIndex,
                                                   const Ui::SignalSlotState &state,
                                                   const Shared::SignalSlotOrder &slotOrder) {
    parameterAccess.writeSlotState(slotIndex, state);
    signalSlotOrderState.setOrder(slotOrder);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) {
    signalSlotOrderState.setOrder(slotOrder);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotColour(const size_t slotIndex, const int colourIndex) {
    parameterAccess.writeSlotColour(slotIndex, colourIndex);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotOpacity(const size_t slotIndex, const float opacity) {
    parameterAccess.writeSlotOpacity(slotIndex, opacity);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setShowPeakEnabled(const bool isEnabled) {
    parameterAccess.writeShowPeak(isEnabled);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setShowRmsEnabled(const bool isEnabled) {
    parameterAccess.writeShowRms(isEnabled);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setShowHoldEnabled(const bool isEnabled) {
    parameterAccess.writeShowHold(isEnabled);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setCustomFrequencyRangeEnabled(const bool isEnabled) {
    parameterAccess.writeUseCustomFrequencyRange(isEnabled);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setVisibleMinFrequencyHz(const float frequencyHz) {
    parameterAccess.writeVisibleMinFrequencyHz(frequencyHz);
    changeTracker->requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::setVisibleMaxFrequencyHz(const float frequencyHz) {
    parameterAccess.writeVisibleMaxFrequencyHz(frequencyHz);
    changeTracker->requestUiRefresh();
}

PluginPresets::PresetActionResult SpectrumAnalyzerAudioProcessor::loadPreset(const PluginPresets::PresetId& presetId) {
    const auto result = presetSession.loadPreset(presetId);
    if (!result.succeeded) {
        publishPresetUiSnapshot();
        return result;
    }

    changeTracker->requestUiRefresh();
    return result;
}

PluginPresets::PresetActionResult SpectrumAnalyzerAudioProcessor::loadPreviousPreset() {
    const auto result = presetSession.loadPreviousPreset();
    if (!result.succeeded) {
        publishPresetUiSnapshot();
        return result;
    }

    changeTracker->requestUiRefresh();
    return result;
}

PluginPresets::PresetActionResult SpectrumAnalyzerAudioProcessor::loadNextPreset() {
    const auto result = presetSession.loadNextPreset();
    if (!result.succeeded) {
        publishPresetUiSnapshot();
        return result;
    }

    changeTracker->requestUiRefresh();
    return result;
}

PluginPresets::PresetActionResult SpectrumAnalyzerAudioProcessor::resetCurrentPreset() {
    const auto result = presetSession.resetCurrentPreset();
    if (!result.succeeded) {
        publishPresetUiSnapshot();
        return result;
    }

    changeTracker->requestUiRefresh();
    return result;
}

PluginPresets::PresetActionResult SpectrumAnalyzerAudioProcessor::savePresetAs(const juce::String& name) {
    const auto result = presetSession.savePresetAs(name);
    publishPresetUiSnapshot();
    return result;
}

PluginPresets::PresetActionResult SpectrumAnalyzerAudioProcessor::overwritePreset(const PluginPresets::PresetId& presetId,
                                                                                  const juce::String& name) {
    const auto result = presetSession.overwritePreset(presetId, name);
    publishPresetUiSnapshot();
    return result;
}

PluginPresets::PresetActionResult SpectrumAnalyzerAudioProcessor::deletePreset(const PluginPresets::PresetId& presetId) {
    const auto result = presetSession.deletePreset(presetId);
    publishPresetUiSnapshot();
    return result;
}

void SpectrumAnalyzerAudioProcessor::refreshPresetCatalog() {
    presetSession.refreshCatalog();
    publishPresetUiSnapshot();
}

void SpectrumAnalyzerAudioProcessor::processorPresetStateChanged() {
    presetSession.markCurrentStateDirty();
}

void SpectrumAnalyzerAudioProcessor::processorUiRefreshRequested() {
    publishAnalyzerUiSnapshot();
    publishEditorPresentationState();
    publishPresetUiSnapshot();
}

void SpectrumAnalyzerAudioProcessor::publishAnalyzerUiSnapshot() {
    const auto snapshot = getAnalyzerUiSnapshot();
    if (lastPublishedUiSnapshot.has_value() && *lastPublishedUiSnapshot == snapshot)
        return;

    lastPublishedUiSnapshot = snapshot;
    uiSnapshotListeners.call([&snapshot](AnalyzerUiSnapshotSource::Listener &listener) {
        listener.analyzerUiSnapshotChanged(snapshot);
    });
}

void SpectrumAnalyzerAudioProcessor::publishEditorPresentationState() {
    const auto state = getEditorPresentationState();
    if (lastPublishedEditorPresentationState.has_value() && *lastPublishedEditorPresentationState == state)
        return;

    lastPublishedEditorPresentationState = state;
    editorPresentationStateListeners.call([&state](EditorPresentationStateSource::Listener &listener) {
        listener.editorPresentationStateChanged(state);
    });
}

void SpectrumAnalyzerAudioProcessor::publishPresetUiSnapshot() {
    const auto snapshot = getPresetUiSnapshot();
    if (lastPublishedPresetUiSnapshot.has_value() && *lastPublishedPresetUiSnapshot == snapshot)
        return;

    lastPublishedPresetUiSnapshot = snapshot;
    presetUiSnapshotListeners.call([&snapshot](PresetUiSnapshotSource::Listener& listener) {
        listener.presetUiSnapshotChanged(snapshot);
    });
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter() {
    return new SpectrumAnalyzerAudioProcessor();
}
