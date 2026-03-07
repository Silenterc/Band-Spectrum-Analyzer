#include "AnalyzerEngine.h"
#include <algorithm>
#include <cmath>

namespace Analyzer {
    namespace {
        constexpr float minAnalysisFrequencyHz = 20.0f;
        // Stay a bit below nyquist so the top band does not get weird near the edge
        constexpr float maxAnalysisFractionOfNyquist = 0.9f;
        constexpr float defaultRmsAttackMs = 15.0f;
        constexpr float defaultRmsReleaseMs = 150.0f;
        constexpr float defaultPeakAttackMs = 3.0f;
        constexpr float defaultPeakReleaseMs = 120.0f;
    }

    void Engine::prepare(double sampleRate, int maximumBlockSize) {
        currentSampleRate = sampleRate;
        currentMaximumBlockSize = maximumBlockSize;
        isPrepared = true;
        summedAnalysisInput.resize(static_cast<size_t>(maximumBlockSize));

        rebuildBands();
        rebuildFilters();
        reset();
    }

    void Engine::reset() {
        const auto floorDb = currentParameters.gridMinDb;

        std::fill(latestFrame.rmsDb.begin(), latestFrame.rmsDb.end(), floorDb);
        std::fill(latestFrame.peakDb.begin(), latestFrame.peakDb.end(), floorDb);
        std::fill(latestFrame.holdDb.begin(), latestFrame.holdDb.end(), floorDb);

        for (auto &band: bands) {
            band.filter.reset();
            band.rmsEnvelope.reset(0.0f);
            band.peakEnvelope.reset(0.0f);
            band.smoothedRmsDb = floorDb;
            band.smoothedPeakDb = floorDb;
            band.heldPeakDb = floorDb;
            band.holdTimeRemainingMs = 0.0f;
        }
    }

    void Engine::setParameters(const ParameterState &parameters) {
        const auto previousBandCount = getBandCount();
        currentParameters = parameters;

        // Band count changes mean we need a whole new bank layout
        if (getBandCount() != previousBandCount)
            rebuildBands();

        rebuildFilters();
    }

    void Engine::processBlock(const juce::AudioBuffer<float> &buffer) {
        if (!isPrepared || bands.empty())
            return;

        const auto floorDb = currentParameters.gridMinDb;

        const auto numInputChannels = buffer.getNumChannels();
        const auto numSamples = static_cast<size_t>(buffer.getNumSamples());

        if (numInputChannels <= 0 || numSamples == 0)
            return;

        const auto blockDurationSeconds = static_cast<float>(numSamples) / static_cast<float>(currentSampleRate);
        const auto blockDurationMs = blockDurationSeconds * 1000.0f;

        switch (currentParameters.analysisMode) {
            case ParamSpec::AnalysisMode::summed:
                processSummedBlock(buffer, floorDb, blockDurationSeconds, blockDurationMs);
                break;
            case ParamSpec::AnalysisMode::midSide:
                processMidSideBlock(buffer, floorDb, blockDurationSeconds, blockDurationMs);
                break;
            case ParamSpec::AnalysisMode::stereo:
                processStereoBlock(buffer, floorDb, blockDurationSeconds, blockDurationMs);
                break;
        }
    }

    const std::vector<BandInfo> &Engine::getBandInfo() const {
        return bandInfo;
    }

    const Frame &Engine::getLatestFrame() const {
        return latestFrame;
    }

    void Engine::rebuildBands() {
        const auto bandCount = getBandCount();
        const auto maxAnalysisFrequencyHz = std::max(
            static_cast<float>(currentSampleRate * 0.5 * maxAnalysisFractionOfNyquist),
            minAnalysisFrequencyHz * 2.0f);
        // This is the ratio we spread evenly in log space
        const auto frequencyRatio = maxAnalysisFrequencyHz / minAnalysisFrequencyHz;

        bandInfo.resize(static_cast<size_t>(bandCount));
        bands.resize(static_cast<size_t>(bandCount));
        latestFrame.rmsDb.resize(static_cast<size_t>(bandCount));
        latestFrame.peakDb.resize(static_cast<size_t>(bandCount));
        latestFrame.holdDb.resize(static_cast<size_t>(bandCount));

        for (int bandIndex = 0; bandIndex < bandCount; ++bandIndex) {
            // We step evenly in log space, not linearly in Hz
            const auto lowNormalised = static_cast<float>(bandIndex) / static_cast<float>(bandCount);
            const auto highNormalised = static_cast<float>(bandIndex + 1) / static_cast<float>(bandCount);
            const auto lowHz = minAnalysisFrequencyHz * std::pow(frequencyRatio, lowNormalised);
            const auto highHz = minAnalysisFrequencyHz * std::pow(frequencyRatio, highNormalised);
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

            band.rmsEnvelope.prepare(processSpec);
            band.rmsEnvelope.setLevelCalculationType(juce::dsp::BallisticsFilterLevelCalculationType::RMS);
            band.rmsEnvelope.setAttackTime(defaultRmsAttackMs);
            band.rmsEnvelope.setReleaseTime(defaultRmsReleaseMs);
            band.rmsEnvelope.reset(0.0f);

            band.peakEnvelope.prepare(processSpec);
            band.peakEnvelope.setLevelCalculationType(juce::dsp::BallisticsFilterLevelCalculationType::peak);
            band.peakEnvelope.setAttackTime(defaultPeakAttackMs);
            band.peakEnvelope.setReleaseTime(defaultPeakReleaseMs);
            band.peakEnvelope.reset(0.0f);
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

    void Engine::processSummedBlock(const juce::AudioBuffer<float> &buffer, float floorDb,
                                    float blockDurationSeconds, float blockDurationMs) {
        const auto numSamples = static_cast<size_t>(buffer.getNumSamples());

        updateSummedAnalysisInput(buffer);

        for (size_t bandIndex = 0; bandIndex < bands.size(); ++bandIndex) {
            auto &band = bands[bandIndex];
            float rmsEnvelopeLinear = 0.0f;
            float peakEnvelopeLinear = 0.0f;

            // Compute the filter, RMS (root mean sq vol) and peak for each band
            for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                // Will potentially need to be revised if this filter is good for our purposes
                const auto filteredSample = band.filter.processSample(0, summedAnalysisInput[sampleIndex]);
                rmsEnvelopeLinear = band.rmsEnvelope.processSample(0, filteredSample);
                peakEnvelopeLinear = band.peakEnvelope.processSample(0, filteredSample);
            }

            updateBandFrame(bandIndex, rmsEnvelopeLinear, peakEnvelopeLinear, floorDb, blockDurationSeconds,
                            blockDurationMs);
        }
    }

    void Engine::processStereoBlock(const juce::AudioBuffer<float> &buffer, float floorDb,
                                    float blockDurationSeconds, float blockDurationMs) {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = static_cast<size_t>(buffer.getNumSamples());
        const auto *leftChannel = buffer.getReadPointer(0);
        // If we only have 1 channel, this becomes the same as summed/mono
        const auto *rightChannel = numChannels > 1 ? buffer.getReadPointer(1) : nullptr;

        for (size_t bandIndex = 0; bandIndex < bands.size(); ++bandIndex) {
            auto &band = bands[bandIndex];
            float leftRmsEnvelopeLinear = 0.0f;
            float rightRmsEnvelopeLinear = 0.0f;
            float leftPeakEnvelopeLinear = 0.0f;
            float rightPeakEnvelopeLinear = 0.0f;

            // Process left and right independently and then merge
            for (size_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
                const auto leftSample = leftChannel[sampleIndex];
                const auto rightSample = rightChannel != nullptr ? rightChannel[sampleIndex] : leftSample;

                const auto leftFilteredSample = band.filter.processSample(0, leftSample);
                const auto rightFilteredSample = band.filter.processSample(1, rightSample);

                leftRmsEnvelopeLinear = band.rmsEnvelope.processSample(0, leftFilteredSample);
                rightRmsEnvelopeLinear = band.rmsEnvelope.processSample(1, rightFilteredSample);
                leftPeakEnvelopeLinear = band.peakEnvelope.processSample(0, leftFilteredSample);
                rightPeakEnvelopeLinear = band.peakEnvelope.processSample(1, rightFilteredSample);
            }

            // Combine channel magnitudes after analysis so we keep stereo energy without phase cancellation
            const auto rmsEnvelopeLinear = 0.5f * (leftRmsEnvelopeLinear + rightRmsEnvelopeLinear);
            const auto peakEnvelopeLinear = 0.5f * (leftPeakEnvelopeLinear + rightPeakEnvelopeLinear);

            updateBandFrame(bandIndex, rmsEnvelopeLinear, peakEnvelopeLinear, floorDb, blockDurationSeconds,
                            blockDurationMs);
        }
    }

    void Engine::processMidSideBlock(const juce::AudioBuffer<float> &buffer, float floorDb,
                                     float blockDurationSeconds, float blockDurationMs) {
        juce::ignoreUnused(buffer, floorDb, blockDurationSeconds, blockDurationMs);

        // TODO implement mid/side analysis path
    }

    void Engine::updateBandFrame(size_t bandIndex, float rmsLinear, float peakLinear, float floorDb,
                                 float blockDurationSeconds, float blockDurationMs) {
        auto &band = bands[bandIndex];

        // Convert to dB before writing the frame
        band.smoothedRmsDb = juce::Decibels::gainToDecibels(rmsLinear, floorDb);
        band.smoothedPeakDb = juce::Decibels::gainToDecibels(peakLinear, floorDb);

        // Update hold if needed
        if (currentParameters.showHold) {
            if (band.smoothedPeakDb >= band.heldPeakDb) {
                // New peak, so pin the hold marker here and restart the timer
                band.heldPeakDb = band.smoothedPeakDb;
                band.holdTimeRemainingMs = currentParameters.holdMs;
            } else if (band.holdTimeRemainingMs > 0.0f) {
                band.holdTimeRemainingMs = std::max(0.0f, band.holdTimeRemainingMs - blockDurationMs);
            } else {
                // Once hold time is over, let it fall at a fixed dB/s rate
                band.heldPeakDb = std::max(band.smoothedPeakDb,
                                           band.heldPeakDb - holdDecayDbPerSecond * blockDurationSeconds);
            }
        }

        latestFrame.rmsDb[bandIndex] = currentParameters.showRms ? band.smoothedRmsDb : floorDb;
        latestFrame.peakDb[bandIndex] = currentParameters.showPeak ? band.smoothedPeakDb : floorDb;
        latestFrame.holdDb[bandIndex] = currentParameters.showHold ? band.heldPeakDb : floorDb;
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
