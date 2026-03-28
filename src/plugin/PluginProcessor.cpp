#include "PluginProcessor.h"
#include "../ui/PluginEditor.h"

namespace {
    // Temporary perf-tuning switch: ignore host/plugin state so new instances always use code defaults.
    constexpr bool bypassStatePersistence = true;
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
      parameterAccess(parameters) {
    parameterAccess.cache();
    registerUiStateParameterListeners();
    signalSlotOrderState.addListener(*this);
}

SpectrumAnalyzerAudioProcessor::~SpectrumAnalyzerAudioProcessor() {
    signalSlotOrderState.removeListener(*this);
    unregisterUiStateParameterListeners();
    cancelPendingUpdate();
}

//==============================================================================
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
    previousEngineParameters = parameterAccess.readEngineState();
    engine.setParameters(*previousEngineParameters);
}

void SpectrumAnalyzerAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void SpectrumAnalyzerAudioProcessor::numBusesChanged() {
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::processorLayoutsChanged() {
    triggerUiStateUpdate();
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

    const auto currentEngineParameters = parameterAccess.readEngineState();

    if (!previousEngineParameters.has_value()
        || *previousEngineParameters != currentEngineParameters)
        engine.setParameters(currentEngineParameters);

    previousEngineParameters = currentEngineParameters;

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    auto mainBuffer = getBusBuffer(buffer, true, 0);

    if (getBusCount(true) > 1 && getChannelCountOfBus(true, 1) > 0) {
        auto sidechainBuffer = getBusBuffer(buffer, true, 1);
        engine.processBlock(mainBuffer, &sidechainBuffer);
        return;
    }

    engine.processBlock(mainBuffer);
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
    if constexpr (bypassStatePersistence) {
        destData.reset();
        return;
    }

    auto state = parameters.copyState();
    signalSlotOrderState.writeTo(state);

    if (const auto stateXml = state.createXml())
        copyXmlToBinary(*stateXml, destData);
}

void SpectrumAnalyzerAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    if constexpr (bypassStatePersistence) {
        juce::ignoreUnused(data, sizeInBytes);
        return;
    }

    const auto stateXml = getXmlFromBinary(data, sizeInBytes);
    if (stateXml == nullptr)
        return;

    const auto state = juce::ValueTree::fromXml(*stateXml);
    if (!state.isValid())
        return;

    signalSlotOrderState.readFrom(state);
    parameters.replaceState(state);
    parameterAccess.cache();

    previousEngineParameters = parameterAccess.readEngineState();
    if (previousEngineParameters.has_value())
        engine.setParameters(*previousEngineParameters);

    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::registerUiStateParameterListeners() {
    parameters.state.addListener(this);
}

void SpectrumAnalyzerAudioProcessor::unregisterUiStateParameterListeners() {
    parameters.state.removeListener(this);
}

void SpectrumAnalyzerAudioProcessor::triggerUiStateUpdate() {
    if (juce::MessageManager::getInstance()->isThisTheMessageThread()) {
        handleAsyncUpdate();
        return;
    }

    triggerAsyncUpdate();
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
    return state;
}

void SpectrumAnalyzerAudioProcessor::addAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) {
    uiSnapshotListeners.add(&listener);
}

void SpectrumAnalyzerAudioProcessor::removeAnalyzerUiSnapshotListener(AnalyzerUiSnapshotSource::Listener &listener) {
    uiSnapshotListeners.remove(&listener);
}

std::vector<Analyzer::RawTrace> SpectrumAnalyzerAudioProcessor::getRawTraces() const {
    return engine.getTraces();
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

juce::AudioProcessorValueTreeState::ParameterLayout SpectrumAnalyzerAudioProcessor::createParameterLayout() {
    return PluginParameters::Schema::makeParameterLayout();
}

void SpectrumAnalyzerAudioProcessor::setFreezeEnabled(const bool isFrozenValue) {
    parameterAccess.writeFreeze(isFrozenValue);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotEnabled(const size_t slotIndex, const bool isEnabled) {
    parameterAccess.writeSlotEnabled(slotIndex, isEnabled);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotVisible(const size_t slotIndex, const bool isVisible) {
    parameterAccess.writeSlotVisible(slotIndex, isVisible);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotFrozen(const size_t slotIndex, const bool isFrozenValue) {
    parameterAccess.writeSlotFrozen(slotIndex, isFrozenValue);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotSource(const size_t slotIndex, const Analyzer::SignalSource source) {
    parameterAccess.writeSlotSignal(slotIndex, source, parameterAccess.readUiSlot(slotIndex).configuration.mode);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotMode(const size_t slotIndex, const Analyzer::SignalMode mode) {
    parameterAccess.writeSlotSignal(slotIndex, parameterAccess.readUiSlot(slotIndex).configuration.source, mode);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotSignal(const size_t slotIndex,
                                                         const Analyzer::SignalSource source,
                                                         const Analyzer::SignalMode mode) {
    parameterAccess.writeSlotSignal(slotIndex, source, mode);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::applySignalSlotState(const size_t slotIndex, const Ui::SignalSlotState &state) {
    parameterAccess.writeSlotState(slotIndex, state);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::removeSignalSlot(const size_t slotIndex) {
    auto state = parameterAccess.readUiSlot(slotIndex);
    state.configuration.enabled = false;
    state.visible = false;
    state.frozen = false;
    parameterAccess.writeSlotState(slotIndex, state);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::addSignalSlot(const size_t slotIndex,
                                                   const Ui::SignalSlotState &state,
                                                   const Shared::SignalSlotOrder &slotOrder) {
    parameterAccess.writeSlotState(slotIndex, state);
    signalSlotOrderState.setOrder(slotOrder);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) {
    signalSlotOrderState.setOrder(slotOrder);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotColour(const size_t slotIndex, const int colourIndex) {
    parameterAccess.writeSlotColour(slotIndex, colourIndex);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotOpacity(const size_t slotIndex, const float opacity) {
    parameterAccess.writeSlotOpacity(slotIndex, opacity);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setShowPeakEnabled(const bool isEnabled) {
    parameterAccess.writeShowPeak(isEnabled);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setShowRmsEnabled(const bool isEnabled) {
    parameterAccess.writeShowRms(isEnabled);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::setShowHoldEnabled(const bool isEnabled) {
    parameterAccess.writeShowHold(isEnabled);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                             const juce::Identifier &property) {
    juce::ignoreUnused(treeWhosePropertyHasChanged, property);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::valueTreeChildAdded(juce::ValueTree &parentTree,
                                                         juce::ValueTree &childWhichHasBeenAdded) {
    juce::ignoreUnused(parentTree, childWhichHasBeenAdded);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::valueTreeChildRemoved(juce::ValueTree &parentTree,
                                                           juce::ValueTree &childWhichHasBeenRemoved,
                                                           const int indexFromWhichChildWasRemoved) {
    juce::ignoreUnused(parentTree, childWhichHasBeenRemoved, indexFromWhichChildWasRemoved);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::valueTreeChildOrderChanged(juce::ValueTree &parentTreeWhoseChildrenHaveMoved,
                                                                const int oldIndex,
                                                                const int newIndex) {
    juce::ignoreUnused(parentTreeWhoseChildrenHaveMoved, oldIndex, newIndex);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::valueTreeParentChanged(juce::ValueTree &treeWhoseParentHasChanged) {
    juce::ignoreUnused(treeWhoseParentHasChanged);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::signalSlotOrderChanged(const Shared::SignalSlotOrder &slotOrder) {
    juce::ignoreUnused(slotOrder);
    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::handleAsyncUpdate() {
    const auto snapshot = getAnalyzerUiSnapshot();
    if (lastPublishedUiSnapshot.has_value() && *lastPublishedUiSnapshot == snapshot)
        return;

    lastPublishedUiSnapshot = snapshot;
    uiSnapshotListeners.call([&snapshot](AnalyzerUiSnapshotSource::Listener &listener) {
        listener.analyzerUiSnapshotChanged(snapshot);
    });
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter() {
    return new SpectrumAnalyzerAudioProcessor();
}
