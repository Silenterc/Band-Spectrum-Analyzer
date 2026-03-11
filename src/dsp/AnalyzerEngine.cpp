#include "AnalyzerEngine.h"
#include "AnalyzerConstants.h"
#include <algorithm>
#include <cmath>

namespace Analyzer {
    void Engine::prepare(double sampleRate, int maximumBlockSize) {
        currentSampleRate = sampleRate;
        currentMaximumBlockSize = maximumBlockSize;
        isPrepared = true;
        sourceBuilder.prepare(maximumBlockSize);

        rebuildBands();
        rebuildProcessors();
        reset();
    }

    void Engine::reset() {
        auto *publishedTraces = traces.get_for_writer();
        if (publishedTraces->size() != publishedTraceCount)
            publishedTraces->resize(publishedTraceCount);

        size_t traceOffset = 0;
        for (auto &processor: processors) {
            const auto outputCount = processor.getOutputCount();
            processor.reset();
            processor.writeRawTraces(*publishedTraces, traceOffset);
            traceOffset += outputCount;
        }

        traces.publish();
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

        const auto sourceSet = sourceBuilder.build(mainBuffer, sidechainBuffer);
        auto *publishedTraces = traces.get_for_writer();
        if (publishedTraces->size() != publishedTraceCount)
            publishedTraces->resize(publishedTraceCount);

        size_t traceOffset = 0;
        for (auto &processor: processors) {
            const auto outputCount = processor.getOutputCount();
            processor.process(sourceSet);
            processor.writeRawTraces(*publishedTraces, traceOffset);
            traceOffset += outputCount;
        }

        traces.publish();
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
}
