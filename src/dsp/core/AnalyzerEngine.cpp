#include "AnalyzerEngine.h"
#include "AnalyzerConstants.h"
#include <algorithm>
#include <cmath>

namespace Analyzer {
    SignalView Engine::sliceSignalView(const SignalView &view, const size_t offset, const size_t numSamples) {
        if (view.data == nullptr || offset >= view.numSamples || numSamples == 0)
            return {};

        const auto availableSamples = view.numSamples - offset;
        const auto sliceLength = std::min(numSamples, availableSamples);
        return {view.data + offset, sliceLength};
    }

    SourceSet Engine::sliceSourceSet(const SourceSet &sourceSet, const size_t offset, const size_t numSamples) {
        SourceSet sliced;
        sliced.mainLeft = sliceSignalView(sourceSet.mainLeft, offset, numSamples);
        sliced.mainRight = sliceSignalView(sourceSet.mainRight, offset, numSamples);
        sliced.mainMid = sliceSignalView(sourceSet.mainMid, offset, numSamples);
        sliced.mainSide = sliceSignalView(sourceSet.mainSide, offset, numSamples);
        sliced.sidechainLeft = sliceSignalView(sourceSet.sidechainLeft, offset, numSamples);
        sliced.sidechainRight = sliceSignalView(sourceSet.sidechainRight, offset, numSamples);
        sliced.sidechainMid = sliceSignalView(sourceSet.sidechainMid, offset, numSamples);
        sliced.sidechainSide = sliceSignalView(sourceSet.sidechainSide, offset, numSamples);
        return sliced;
    }

    void Engine::prepare(double sampleRate, int maximumBlockSize) {
        currentSampleRate = sampleRate;
        currentMaximumBlockSize = maximumBlockSize;
        isPrepared = true;
        sourceBuilder.prepare(maximumBlockSize);
        inputActivityDetector.prepare(sampleRate);

        rebuildBands();
        rebuildProcessors();
        reset();
    }

    void Engine::reset() {
        inputActivityDetector.reset();
        frameSlots = {};
        frameSlots[0].active = true;
        nextFrameSlotToStart = 1;
        samplesUntilNextFrameStart = Constants::analysisHopSamples;
        recentSignalActive.store(false, std::memory_order_relaxed);
        hasPublishedSilenceWhileInactive = false;
        publishProcessorState(true);
    }

    void Engine::setParameters(const EngineParameterState &parameters) {
        const auto previousBandCount = getBandCount();
        currentParameters = parameters;

        // Band count changes mean we need a whole new bank layout
        if (getBandCount() != previousBandCount)
            rebuildBands();

        rebuildProcessors();
        reset();
    }

    void Engine::processBlock(const juce::AudioBuffer<float> &buffer) {
        processBlock(buffer, nullptr);
    }

    void Engine::processBlock(const juce::AudioBuffer<float> &mainBuffer,
                              const juce::AudioBuffer<float> *sidechainBuffer) {
        if (!isPrepared)
            return;

        const auto numInputChannels = mainBuffer.getNumChannels();
        const auto numSamples = mainBuffer.getNumSamples();

        if (numInputChannels <= 0 || numSamples == 0)
            return;

        inputActivityDetector.update(mainBuffer, sidechainBuffer);
        recentSignalActive.store(inputActivityDetector.hasRecentSignal(), std::memory_order_relaxed);

        if (!inputActivityDetector.shouldProcess()) {
            if (!hasPublishedSilenceWhileInactive) {
                frameSlots = {};
                frameSlots[0].active = true;
                nextFrameSlotToStart = 1;
                samplesUntilNextFrameStart = Constants::analysisHopSamples;
                publishProcessorState(true);
                hasPublishedSilenceWhileInactive = true;
            }
            return;
        }

        hasPublishedSilenceWhileInactive = false;
        const auto sourceSet = sourceBuilder.build(mainBuffer, sidechainBuffer);
        size_t processedSamples = 0;
        while (processedSamples < static_cast<size_t>(numSamples)) {
            const auto sliceSamples = std::min(static_cast<size_t>(numSamples) - processedSamples, samplesUntilNextFrameStart);
            const auto slicedSources = sliceSourceSet(sourceSet, processedSamples, sliceSamples);

            for (auto &processor: processors) {
                processor.process(slicedSources);
                for (size_t frameSlotIndex = 0; frameSlotIndex < frameSlots.size(); ++frameSlotIndex) {
                    if (frameSlots[frameSlotIndex].active)
                        processor.accumulateCurrentSlice(frameSlotIndex);
                }
            }

            for (auto &frameSlot: frameSlots) {
                if (frameSlot.active)
                    frameSlot.fillSamples += sliceSamples;
            }

            processedSamples += sliceSamples;
            samplesUntilNextFrameStart -= sliceSamples;

            if (samplesUntilNextFrameStart == 0) {
                const auto slotToRestart = nextFrameSlotToStart;

                if (frameSlots[slotToRestart].active
                    && frameSlots[slotToRestart].fillSamples == Constants::analysisFrameSamples) {
                    publishProcessorState(false, slotToRestart);
                    for (auto &processor: processors)
                        processor.clearAccumulatedFrame(slotToRestart);
                }

                frameSlots[slotToRestart].active = true;
                frameSlots[slotToRestart].fillSamples = 0;
                nextFrameSlotToStart = (nextFrameSlotToStart + 1) % frameSlots.size();
                samplesUntilNextFrameStart = Constants::analysisHopSamples;
            }
        }
    }

    std::shared_ptr<const std::vector<BandInfo>> Engine::getBandInfo() const {
        return std::atomic_load(&bandInfo);
    }

    std::vector<RawTrace> Engine::getTraces() const {
        // TripleBuffer gives us the latest published raw traces without touching the live write side
        const auto [publishedTraces, hasUpdate] = traces.get_for_reader();
        juce::ignoreUnused(hasUpdate);
        return *publishedTraces;
    }

    bool Engine::hasRecentSignal() const {
        return recentSignalActive.load(std::memory_order_relaxed);
    }

    void Engine::rebuildBands() {
        const auto bandCount = static_cast<size_t>(getBandCount());
        const auto maxAnalysisFrequencyHz = std::max(
            static_cast<float>(currentSampleRate * 0.5 * Constants::maxAnalysisFractionOfNyquist),
            Constants::minFrequencyHz * 2.0f);
        // This is the ratio we spread evenly in log space
        const auto frequencyRatio = maxAnalysisFrequencyHz / Constants::minFrequencyHz;
        const auto normalisedStep = 1.0f / static_cast<float>(bandCount);

        auto newBandInfo = std::make_shared<std::vector<BandInfo> >();
        newBandInfo->resize(bandCount);

        auto lowNormalised = 0.0f;
        for (size_t bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
            // We step evenly in log space, not linearly in Hz
            const auto highNormalised = lowNormalised + normalisedStep;
            const auto lowHz = Constants::minFrequencyHz * std::pow(frequencyRatio, lowNormalised);
            const auto highHz = Constants::minFrequencyHz * std::pow(frequencyRatio, highNormalised);
            // Geometric mean is the natural center for a log-spaced band
            const auto centerHz = std::sqrt(lowHz * highHz);

            BandInfo info{};
            info.lowHz = lowHz;
            info.centerHz = centerHz;
            info.highHz = highHz;

            (*newBandInfo)[bandIndex] = info;
            lowNormalised = highNormalised;
        }

        std::atomic_store(&bandInfo, newBandInfo);
    }

    void Engine::rebuildProcessors() {
        if (!isPrepared)
            return;

        processors.clear();
        publishedTraceCount = 0;

        const auto specs = planBuilder.build(currentParameters);
        const auto currentBandInfo = getBandInfo();
        processors.reserve(specs.size());

        for (const auto &spec: specs) {
            processors.emplace_back(spec);
            processors.back().prepare(currentSampleRate, currentMaximumBlockSize, currentBandInfo);
            publishedTraceCount += processors.back().getOutputCount();
        }
    }

    int Engine::getBandCount() const {
        switch (currentParameters.bandMode) {
            case BandMode::bands30:
                return 30;
            case BandMode::bands45:
                return 45;
            case BandMode::bands60:
                return 60;
        }

        return 45;
    }

    void Engine::publishProcessorState(const bool resetProcessors, const size_t frameSlotIndex) {
        auto *publishedTraces = traces.get_for_writer();
        if (publishedTraces->size() != publishedTraceCount)
            publishedTraces->resize(publishedTraceCount);

        size_t traceOffset = 0;
        for (auto &processor: processors) {
            const auto outputCount = processor.getOutputCount();
            if (resetProcessors)
                processor.reset();
            processor.writeRawTraces(*publishedTraces, traceOffset, frameSlotIndex);
            traceOffset += outputCount;
        }

        traces.publish();
    }
}
