#include "AnalyzerEngine.h"
#include "AnalyzerConstants.h"
#include <algorithm>
#include <cmath>

namespace Analyzer {
    void Engine::prepare(double sampleRate, int maximumBlockSize) {
        currentSampleRate = sampleRate;
        currentMaximumBlockSize = maximumBlockSize;
        isPrepared = true;
        summedAnalysisInput.resize(static_cast<size_t>(maximumBlockSize));

        rebuildBands();
        rebuildFilters();
        reset();
        publishSnapshot();
    }

    void Engine::reset() {
        clearMeasurements();

        for (auto &band: bands) {
            band.filter.reset();
        }

        publishSnapshot();
    }

    void Engine::setParameters(const ParameterState &parameters) {
        const auto previousBandCount = getBandCount();
        currentParameters = parameters;

        // Band count changes mean we need a whole new bank layout
        if (getBandCount() != previousBandCount)
            rebuildBands();

        rebuildFilters();
        publishSnapshot();
    }

    void Engine::processBlock(const juce::AudioBuffer<float> &buffer) {
        if (!isPrepared || bands.empty())
            return;

        const auto numInputChannels = buffer.getNumChannels();
        const auto numSamples = static_cast<size_t>(buffer.getNumSamples());

        if (numInputChannels <= 0 || numSamples == 0)
            return;

        clearMeasurements();

        switch (currentParameters.analysisMode) {
            case ParamSpec::AnalysisMode::summed:
                processSummedBlock(buffer);
                break;
            case ParamSpec::AnalysisMode::midSide:
                processMidSideBlock(buffer);
                break;
            case ParamSpec::AnalysisMode::stereo:
                processStereoBlock(buffer);
                break;
        }

        publishSnapshot();
    }

    RawSnapshot Engine::getSnapshot() const {
        const auto [snapshot, hasUpdate] = snapshots.get_for_reader();
        juce::ignoreUnused(hasUpdate);
        return *snapshot;
    }

    void Engine::rebuildBands() {
        const auto bandCount = getBandCount();
        const auto maxAnalysisFrequencyHz = std::max(
            static_cast<float>(currentSampleRate * 0.5 * Constants::maxAnalysisFractionOfNyquist),
            Constants::minFrequencyHz * 2.0f);
        // This is the ratio we spread evenly in log space
        const auto frequencyRatio = maxAnalysisFrequencyHz / Constants::minFrequencyHz;

        bandInfo.resize(static_cast<size_t>(bandCount));
        bands.resize(static_cast<size_t>(bandCount));
        latestMeasurements.resize(static_cast<size_t>(bandCount));

        for (int bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
            // We step evenly in log space, not linearly in Hz
            const auto lowNormalised = static_cast<float>(bandIndex) / static_cast<float>(bandCount);
            const auto highNormalised = static_cast<float>(bandIndex + 1) / static_cast<float>(bandCount);
            const auto lowHz = Constants::minFrequencyHz * std::pow(frequencyRatio, lowNormalised);
            const auto highHz = Constants::minFrequencyHz * std::pow(frequencyRatio, highNormalised);
            // Geometric mean is the natural center for a log-spaced band
            const auto centerHz = std::sqrt(lowHz * highHz);

            BandInfo info{};
            info.lowHz = lowHz;
            info.centerHz = centerHz;
            info.highHz = highHz;

            bandInfo[static_cast<size_t>(bandIndex)] = info;
            bands[static_cast<size_t>(bandIndex)].info = info;
        }
    }

    void Engine::rebuildFilters() {
        if (!isPrepared)
            return;

        juce::dsp::ProcessSpec processSpec{};
        processSpec.sampleRate = currentSampleRate;
        processSpec.maximumBlockSize = static_cast<juce::uint32>(currentMaximumBlockSize);
        processSpec.numChannels = 2;

        for (auto &band: bands) {
            const auto bandwidth = std::max(band.info.highHz - band.info.lowHz, 1.0f);
            // Rough q from the band width. Good enough until we calibrate this more carefully
            // Higher resonance means a narrower, more focused band around the center freq
            const auto resonance = juce::jlimit(0.1f, 10.0f, band.info.centerHz / bandwidth);

            band.filter.prepare(processSpec);
            band.filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
            band.filter.setCutoffFrequency(band.info.centerHz);
            band.filter.setResonance(resonance);
            band.filter.reset();
        }
    }

    void Engine::updateSummedAnalysisInput(const juce::AudioBuffer<float> &buffer) {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = static_cast<size_t>(buffer.getNumSamples());
        const auto *leftChannel = buffer.getReadPointer(0);
        const auto *rightChannel = numChannels > 1 ? buffer.getReadPointer(1) : nullptr;
        summedAnalysisInput.resize(numSamples);

        // Precompute the mono sum once so each band can reuse it
        for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
            const auto leftSample = leftChannel[sampleIndex];
            const auto rightSample = rightChannel != nullptr ? rightChannel[sampleIndex] : leftSample;
            summedAnalysisInput[sampleIndex] = 0.5f * (leftSample + rightSample);
        }
    }

    void Engine::processSummedBlock(const juce::AudioBuffer<float> &buffer) {
        const auto numSamples = static_cast<size_t>(buffer.getNumSamples());

        updateSummedAnalysisInput(buffer);

        for (size_t bandIndex = 0; bandIndex < bands.size(); ++bandIndex) {
            auto &band = bands[bandIndex];
            auto &measurements = latestMeasurements[bandIndex];

            // Measure raw power on the audio side. Display smoothing happens later at poll rate
            for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                // Will potentially need to be revised if this filter is good for our purposes
                const auto filteredSample = band.filter.processSample(0, summedAnalysisInput[sampleIndex]);
                const auto samplePower = filteredSample * filteredSample;
                measurements.peakPower = std::max(measurements.peakPower, samplePower);
                measurements.sumPower += static_cast<double>(samplePower);
                ++measurements.numSamples;
            }
        }
    }

    void Engine::processStereoBlock(const juce::AudioBuffer<float> &buffer) {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = static_cast<size_t>(buffer.getNumSamples());
        const auto *leftChannel = buffer.getReadPointer(0);
        // If we only have 1 channel, this becomes the same as summed/mono
        const auto *rightChannel = numChannels > 1 ? buffer.getReadPointer(1) : nullptr;

        for (size_t bandIndex = 0; bandIndex < bands.size(); ++bandIndex) {
            auto &band = bands[bandIndex];
            auto &measurements = latestMeasurements[bandIndex];

            // Process left and right independently and merge in the power domain
            for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                const auto leftSample = leftChannel[sampleIndex];
                const auto rightSample = rightChannel != nullptr ? rightChannel[sampleIndex] : leftSample;

                const auto leftFilteredSample = band.filter.processSample(0, leftSample);
                const auto rightFilteredSample = band.filter.processSample(1, rightSample);
                const auto leftPower = leftFilteredSample * leftFilteredSample;
                const auto rightPower = rightFilteredSample * rightFilteredSample;
                const auto combinedPower = 0.5f * (leftPower + rightPower);

                measurements.peakPower = std::max(measurements.peakPower, combinedPower);
                measurements.sumPower += static_cast<double>(combinedPower);
                ++measurements.numSamples;
            }
        }
    }

    void Engine::processMidSideBlock(const juce::AudioBuffer<float> &buffer) {
        juce::ignoreUnused(buffer);

        // TODO implement mid/side analysis path
    }

    void Engine::clearMeasurements() {
        for (auto &measurements: latestMeasurements) {
            measurements.peakPower = 0.0f;
            measurements.sumPower = 0.0;
            measurements.numSamples = 0;
        }
    }

    void Engine::publishSnapshot() const {
        auto *snapshot = snapshots.get_for_writer();
        snapshot->bandInfo = bandInfo;
        snapshot->traces.resize(1);
        snapshot->traces.front().kind = TraceKind::input;
        snapshot->traces.front().measurements = latestMeasurements;
        snapshots.publish();
    }

    int Engine::getBandCount() const {
        switch (currentParameters.bandMode) {
            case ParamSpec::BandMode::bands30:
                return 30;
            case ParamSpec::BandMode::bands40:
                return 40;
            case ParamSpec::BandMode::bands60:
                return 60;
        }

        return 40;
    }
}
