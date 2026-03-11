#include "PluginProcessor.h"
#include "../ui/PluginEditor.h"

namespace {
    template <typename Enum>
    Enum loadEnumParameter(const std::atomic<float> *parameter) {
        return static_cast<Enum>(juce::roundToInt(parameter->load()));
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
      parameters(*this, nullptr, "SpecParams", createParameterLayout()) {
    cacheParameterPointers();
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
    previousEngineParameters = readCurrentEngineParameters();
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

    const auto currentEngineParameters = readCurrentEngineParameters();

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
    auto state = parameters.copyState();
    signalSlotOrderState.writeTo(state);

    if (const auto stateXml = state.createXml())
        copyXmlToBinary(*stateXml, destData);
}

void SpectrumAnalyzerAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    const auto stateXml = getXmlFromBinary(data, sizeInBytes);
    if (stateXml == nullptr)
        return;

    const auto state = juce::ValueTree::fromXml(*stateXml);
    if (!state.isValid())
        return;

    signalSlotOrderState.readFrom(state);
    parameters.replaceState(state);
    cacheParameterPointers();

    previousEngineParameters = readCurrentEngineParameters();
    if (previousEngineParameters.has_value())
        engine.setParameters(*previousEngineParameters);

    triggerUiStateUpdate();
}

void SpectrumAnalyzerAudioProcessor::cacheParameterPointers() {
    bandModeParam = parameters.getRawParameterValue(ParamIDs::bandMode);
    freezeParam = parameters.getRawParameterValue(ParamIDs::freeze);

    for (size_t slotIndex = 0; slotIndex < signalSlotParams.size(); ++slotIndex) {
        signalSlotParams[slotIndex].enabled = parameters.getRawParameterValue(ParamIDs::signalSlotEnabled(static_cast<int>(slotIndex)));
        signalSlotParams[slotIndex].visible = parameters.getRawParameterValue(ParamIDs::signalSlotVisible(static_cast<int>(slotIndex)));
        signalSlotParams[slotIndex].source = parameters.getRawParameterValue(ParamIDs::signalSlotSource(static_cast<int>(slotIndex)));
        signalSlotParams[slotIndex].mode = parameters.getRawParameterValue(ParamIDs::signalSlotMode(static_cast<int>(slotIndex)));
        signalSlotParams[slotIndex].colour = parameters.getRawParameterValue(ParamIDs::signalSlotColour(static_cast<int>(slotIndex)));
        signalSlotParams[slotIndex].opacity = parameters.getRawParameterValue(ParamIDs::signalSlotOpacity(static_cast<int>(slotIndex)));
    }

    showRmsParam = parameters.getRawParameterValue(ParamIDs::showRms);
    showPeakParam = parameters.getRawParameterValue(ParamIDs::showPeak);
    showHoldParam = parameters.getRawParameterValue(ParamIDs::showHold);

    holdMsParam = parameters.getRawParameterValue(ParamIDs::holdMs);
    gridMinDbParam = parameters.getRawParameterValue(ParamIDs::gridMinDb);
    gridMaxDbParam = parameters.getRawParameterValue(ParamIDs::gridMaxDb);
    gridStepDbParam = parameters.getRawParameterValue(ParamIDs::gridStepDb);
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

Ui::AnalyzerUiState SpectrumAnalyzerAudioProcessor::getAnalyzerUiState() const {
    Ui::AnalyzerUiState state;
    state.signalSlots = getSignalSlots();
    state.slotOrder = getSignalSlotOrder();
    state.meterSettings = getMeterSettings();
    state.frozen = isFrozen();
    state.sidechainAvailable = isSidechainAvailable();
    return state;
}

void SpectrumAnalyzerAudioProcessor::addAnalyzerUiStateListener(AnalyzerUiStateSource::Listener &listener) {
    uiStateListeners.add(&listener);
}

void SpectrumAnalyzerAudioProcessor::removeAnalyzerUiStateListener(AnalyzerUiStateSource::Listener &listener) {
    uiStateListeners.remove(&listener);
}

std::vector<Analyzer::RawTrace> SpectrumAnalyzerAudioProcessor::getRawTraces() const {
    return engine.getTraces();
}

std::array<Ui::SignalSlotState, Shared::maxSignalSlots> SpectrumAnalyzerAudioProcessor::getSignalSlots() const {
    std::array<Ui::SignalSlotState, Shared::maxSignalSlots> signalSlots{};

    for (size_t slotIndex = 0; slotIndex < signalSlots.size(); ++slotIndex) {
        auto &slot = signalSlots[slotIndex];
        const auto &slotParams = signalSlotParams[slotIndex];
        slot.configuration.enabled = slotParams.enabled->load() > 0.5f;
        slot.visible = slotParams.visible->load() > 0.5f;
        slot.configuration.source = loadEnumParameter<Analyzer::SignalSource>(slotParams.source);
        slot.configuration.mode = loadEnumParameter<Analyzer::SignalMode>(slotParams.mode);
        slot.colourIndex = juce::roundToInt(slotParams.colour->load());
        slot.opacity = slotParams.opacity->load();
    }

    return signalSlots;
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
    return freezeParam->load() > 0.5f;
}

Analyzer::MeterSettings SpectrumAnalyzerAudioProcessor::getMeterSettings() const {
    Analyzer::MeterSettings meterSettings;
    meterSettings.showRms = showRmsParam->load() > 0.5f;
    meterSettings.showPeak = showPeakParam->load() > 0.5f;
    meterSettings.showHold = showHoldParam->load() > 0.5f;
    meterSettings.holdMs = holdMsParam->load();
    return meterSettings;
}

float SpectrumAnalyzerAudioProcessor::getGridMinDb() const {
    return gridMinDbParam->load();
}

float SpectrumAnalyzerAudioProcessor::getGridMaxDb() const {
    return gridMaxDbParam->load();
}

float SpectrumAnalyzerAudioProcessor::getGridStepDb() const {
    return gridStepDbParam->load();
}

Analyzer::EngineParameterState SpectrumAnalyzerAudioProcessor::readCurrentEngineParameters() const {
    Analyzer::EngineParameterState currentEngineParameters;
    currentEngineParameters.bandMode = loadEnumParameter<Analyzer::BandMode>(bandModeParam);

    for (size_t slotIndex = 0; slotIndex < currentEngineParameters.signalSlots.size(); ++slotIndex) {
        const auto &slotParams = signalSlotParams[slotIndex];
        auto &slot = currentEngineParameters.signalSlots[slotIndex];
        slot.enabled = slotParams.enabled->load() > 0.5f;
        slot.source = loadEnumParameter<Analyzer::SignalSource>(slotParams.source);
        slot.mode = loadEnumParameter<Analyzer::SignalMode>(slotParams.mode);
    }

    return currentEngineParameters;
}

juce::AudioProcessorValueTreeState::ParameterLayout SpectrumAnalyzerAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(ParamSpec::makeBandModeParameter());
    layout.add(ParamSpec::makeFreezeParameter());

    for (size_t slotIndex = 0; slotIndex < Shared::maxSignalSlots; ++slotIndex) {
        const auto defaultEnabled = slotIndex == 0;
        const auto defaultVisible = true;
        const auto defaultSource = Analyzer::SignalSource::main;
        const auto defaultMode = slotIndex == 0 ? Analyzer::SignalMode::stereo : Analyzer::SignalMode::mid;
        const auto defaultColour = static_cast<int>(slotIndex);
        layout.add(ParamSpec::makeSignalSlotEnabledParameter(static_cast<int>(slotIndex), defaultEnabled));
        layout.add(ParamSpec::makeSignalSlotVisibleParameter(static_cast<int>(slotIndex), defaultVisible));
        layout.add(ParamSpec::makeSignalSlotSourceParameter(static_cast<int>(slotIndex), defaultSource));
        layout.add(ParamSpec::makeSignalSlotModeParameter(static_cast<int>(slotIndex), defaultMode));
        layout.add(ParamSpec::makeSignalSlotColourParameter(static_cast<int>(slotIndex), defaultColour));
        layout.add(ParamSpec::makeSignalSlotOpacityParameter(static_cast<int>(slotIndex), Ui::defaultSignalOpacity));
    }

    layout.add(ParamSpec::makeShowRmsParameter());
    layout.add(ParamSpec::makeShowPeakParameter());
    layout.add(ParamSpec::makeShowHoldParameter());

    layout.add(ParamSpec::makeHoldMsParameter());
    layout.add(ParamSpec::makeGridMinDbParameter());
    layout.add(ParamSpec::makeGridMaxDbParameter());
    layout.add(ParamSpec::makeGridStepDbParameter());

    return layout;
}

void SpectrumAnalyzerAudioProcessor::setFreezeEnabled(const bool isFrozenValue) {
    setBoolParameter(ParamIDs::freeze, isFrozenValue);
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotEnabled(const size_t slotIndex, const bool isEnabled) {
    setBoolParameter(ParamIDs::signalSlotEnabled(static_cast<int>(slotIndex)), isEnabled);
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotVisible(const size_t slotIndex, const bool isVisible) {
    setBoolParameter(ParamIDs::signalSlotVisible(static_cast<int>(slotIndex)), isVisible);
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotSource(const size_t slotIndex, const Analyzer::SignalSource source) {
    setChoiceParameter(ParamIDs::signalSlotSource(static_cast<int>(slotIndex)), static_cast<int>(source), 2);
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotMode(const size_t slotIndex, const Analyzer::SignalMode mode) {
    setChoiceParameter(ParamIDs::signalSlotMode(static_cast<int>(slotIndex)), static_cast<int>(mode), 3);
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotOrder(const Shared::SignalSlotOrder &slotOrder) {
    signalSlotOrderState.setOrder(slotOrder);
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotColour(const size_t slotIndex, const int colourIndex) {
    setChoiceParameter(ParamIDs::signalSlotColour(static_cast<int>(slotIndex)), colourIndex, Ui::signalPresetCount);
}

void SpectrumAnalyzerAudioProcessor::setSignalSlotOpacity(const size_t slotIndex, const float opacity) {
    setFloatParameter(ParamIDs::signalSlotOpacity(static_cast<int>(slotIndex)),
                      juce::jlimit(0.15f, 1.0f, opacity));
}

void SpectrumAnalyzerAudioProcessor::setShowPeakEnabled(const bool isEnabled) {
    setBoolParameter(ParamIDs::showPeak, isEnabled);
}

void SpectrumAnalyzerAudioProcessor::setShowRmsEnabled(const bool isEnabled) {
    setBoolParameter(ParamIDs::showRms, isEnabled);
}

void SpectrumAnalyzerAudioProcessor::setShowHoldEnabled(const bool isEnabled) {
    setBoolParameter(ParamIDs::showHold, isEnabled);
}

void SpectrumAnalyzerAudioProcessor::setBoolParameter(const juce::String &parameterID, const bool value) {
    if (auto *parameter = parameters.getParameter(parameterID)) {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(value ? 1.0f : 0.0f);
        parameter->endChangeGesture();
    }
}

void SpectrumAnalyzerAudioProcessor::setChoiceParameter(const juce::String &parameterID,
                                                        const int choiceIndex,
                                                        const int choiceCount) {
    if (auto *parameter = parameters.getParameter(parameterID)) {
        const auto normalisedValue = choiceCount > 1
                                         ? static_cast<float>(choiceIndex) / static_cast<float>(choiceCount - 1)
                                         : 0.0f;
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(normalisedValue);
        parameter->endChangeGesture();
    }
}

void SpectrumAnalyzerAudioProcessor::setFloatParameter(const juce::String &parameterID, const float plainValue) {
    if (auto *parameter = parameters.getParameter(parameterID)) {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(plainValue));
        parameter->endChangeGesture();
    }
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
    const auto state = getAnalyzerUiState();
    if (lastPublishedUiState.has_value() && *lastPublishedUiState == state)
        return;

    lastPublishedUiState = state;
    uiStateListeners.call([&state](AnalyzerUiStateSource::Listener &listener) {
        listener.analyzerUiStateChanged(state);
    });
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter() {
    return new SpectrumAnalyzerAudioProcessor();
}
