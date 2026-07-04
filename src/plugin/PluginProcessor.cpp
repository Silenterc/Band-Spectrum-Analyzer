#include "PluginProcessor.h"

#include "../ui/editor/contracts/EditorContext.h"
#include "../ui/editor/view/PluginEditor.h"

namespace {
    constexpr auto selectedPresetIdProperty = "__selectedPresetId";
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
                    userPresetStore),
      uiBridge(parameterAccess,
               signalSlotOrderState,
               presetSession,
               [this] { return isSidechainAvailable(); }) {
    parameterAccess.cache();
    changeTracker = std::make_unique<ProcessorChangeTracker>(parameters, signalSlotOrderState, uiBridge);
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
    uiBridge.requestUiRefresh();
}

void SpectrumAnalyzerAudioProcessor::processorLayoutsChanged() {
    uiBridge.requestUiRefresh();
}

bool SpectrumAnalyzerAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

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

    // Output channels without matching inputs are not guaranteed to be empty,
    // so clear them to avoid feedback from garbage samples.
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

    auto mainBuffer = getBusBuffer(buffer, true, 0);
    const auto mixerSlotStates = parameterAccess.readMixerSlots();

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
    return true;
}

juce::AudioProcessorEditor *SpectrumAnalyzerAudioProcessor::createEditor() {
    const Ui::EditorContext context{
        .rawTraceSource = engine,
        .analyzerUiSnapshotSource = uiBridge,
        .presetUiSnapshotSource = uiBridge,
        .analyzerSettingsActions = uiBridge,
        .presetActions = uiBridge,
        .editorPresentationStateSource = uiBridge
    };
    return new SpectrumAnalyzerAudioProcessorEditor(*this, context);
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

    std::optional<Ui::Presets::PresetId> restoredSelectedPresetId;
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
    uiBridge.requestUiRefresh();
}

bool SpectrumAnalyzerAudioProcessor::isSidechainAvailable() const {
    return getBusCount(true) > 1 && getChannelCountOfBus(true, 1) > 0;
}

juce::AudioProcessorValueTreeState::ParameterLayout SpectrumAnalyzerAudioProcessor::createParameterLayout() {
    return PluginParameters::Schema::makeParameterLayout();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter() {
    return new SpectrumAnalyzerAudioProcessor();
}
