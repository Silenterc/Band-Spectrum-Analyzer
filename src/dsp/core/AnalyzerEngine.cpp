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
        hopDurationSeconds = static_cast<float>(static_cast<double>(Constants::analysisHopSamples) / currentSampleRate);
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
        processAnalyzerActive.store(false, std::memory_order_relaxed);
        hasPublishedSilenceWhileInactive = false;
        publishProcessorState(true);
    }

    void Engine::setParameters(const EngineParameterState &parameters) {
        const auto didBandModeChange = currentParameters.bandMode != parameters.bandMode;
        currentParameters = parameters;

        // Fractional-octave mode changes mean we need a whole new band layout,
        // even if two modes would ever produce the same total count.
        if (didBandModeChange)
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
        processAnalyzerActive.store(inputActivityDetector.shouldProcess(), std::memory_order_relaxed);
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
                // Accumulate this slice into every overlapping analysis frame that is currently active
                for (size_t frameSlotIndex = 0; frameSlotIndex < frameSlots.size(); ++frameSlotIndex) {
                    if (frameSlots[frameSlotIndex].active)
                        processor.accumulateCurrentSlice(frameSlotIndex);
                }
                processor.accumulateCurrentHop();
            }

            for (auto &frameSlot: frameSlots) {
                if (frameSlot.active)
                    frameSlot.fillSamples += sliceSamples;
            }

            processedSamples += sliceSamples;
            samplesUntilNextFrameStart -= sliceSamples;

            if (samplesUntilNextFrameStart == 0) {
                // Reuse the next rotating frame slot at each hop boundary.
                const auto slotToRestart = nextFrameSlotToStart;
                const auto completedFrame = frameSlots[slotToRestart].active
                                           && frameSlots[slotToRestart].fillSamples == Constants::analysisFrameSamples;

                // Publish once per hop. Peak comes from the completed overlapping frame when available,
                // while RMS always comes from the newly advanced non-overlapping hop.
                publishProcessorState(false, slotToRestart);

                for (auto &processor: processors) {
                    if (completedFrame) {
                        processor.clearAccumulatedFrame(slotToRestart);
                    }
                    processor.clearCurrentHop();
                }

                // Start a fresh overlapping frame in this slot.
                frameSlots[slotToRestart].active = true;
                frameSlots[slotToRestart].fillSamples = 0;
                // Advance to the other slot for the next hop.
                nextFrameSlotToStart = (nextFrameSlotToStart + 1) % frameSlots.size();
                // Reset the countdown until the next hop boundary.
                samplesUntilNextFrameStart = Constants::analysisHopSamples;
            }
        }
    }

    std::shared_ptr<const std::vector<BandInfo>> Engine::getBandInfo() const {
        return std::atomic_load(&bandInfo);
    }

    AnalyzerPublishedTracesView Engine::readPublishedTraces() const {
        const auto [publishedTraces, hasUpdate] = traces.get_for_reader();
        return {.traces = publishedTraces, .hasUpdate = hasUpdate, .hopDurationSeconds = hopDurationSeconds};
    }

    bool Engine::hasRecentSignal() const {
        return recentSignalActive.load(std::memory_order_relaxed);
    }

    bool Engine::shouldProcessAnalyzer() const {
        return processAnalyzerActive.load(std::memory_order_relaxed);
    }

    void Engine::rebuildBands() {
        auto newBandInfo = std::make_shared<std::vector<BandInfo> >();
        const auto maxAnalysisFrequencyHz = std::max(
            currentSampleRate * 0.5 * static_cast<double>(Constants::maxAnalysisFractionOfNyquist),
            static_cast<double>(Constants::minFrequencyHz * 2.0f));
        const auto centerRatio = std::pow(2.0, 1.0 / static_cast<double>(getBandsPerOctave()));
        const auto edgeRatio = std::sqrt(centerRatio);
        auto centerHz = Constants::equalTemperedAnchorHz;

        while (centerHz / edgeRatio < static_cast<double>(Constants::minFrequencyHz))
            centerHz *= centerRatio;

        while (true) {
            const auto previousCenterHz = centerHz / centerRatio;
            const auto previousLowHz = previousCenterHz / edgeRatio;
            if (previousLowHz < static_cast<double>(Constants::minFrequencyHz))
                break;

            centerHz = previousCenterHz;
        }

        while (true) {
            const auto lowHz = centerHz / edgeRatio;
            const auto highHz = centerHz * edgeRatio;
            if (highHz > maxAnalysisFrequencyHz)
                break;

            BandInfo info{};
            info.lowHz = static_cast<float>(lowHz);
            info.centerHz = static_cast<float>(centerHz);
            info.highHz = static_cast<float>(highHz);
            newBandInfo->push_back(info);

            centerHz *= centerRatio;
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

    int Engine::getBandsPerOctave() const {
        switch (currentParameters.bandMode) {
            case BandMode::octaveThird:
                return 3;
            case BandMode::octaveQuarter:
                return 4;
            case BandMode::octaveSixth:
                return 6;
            case BandMode::octaveTwelfth:
                return 12;
        }

        return 6;
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
