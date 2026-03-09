#include <algorithm>
#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "dsp/AnalyzerEngine.h"

namespace {
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 1024;
    constexpr int settleBlocks = 24;
    constexpr float testAmplitude = 0.8f;
    constexpr float lowSineHz = 80.0f;
    constexpr float highSineHz = 6000.0f;

    struct TestTone {
        juce::dsp::Oscillator<float> oscillator;
        float polarity = 1.0f;
    };

    Analyzer::ParameterState makeDefaultParameters() {
        Analyzer::ParameterState parameters;
        parameters.analysisMode = ParamSpec::AnalysisMode::summed;
        parameters.bandMode = ParamSpec::BandMode::bands40;
        parameters.showRms = true;
        parameters.showPeak = true;
        parameters.showHold = false;
        parameters.holdMs = ParamSpec::defaultHoldMs;
        parameters.gridMinDb = ParamSpec::defaultGridMinDb;
        parameters.gridMaxDb = ParamSpec::defaultGridMaxDb;
        parameters.gridStepDb = ParamSpec::defaultGridStepDb;
        return parameters;
    }

    void prepareEngine(Analyzer::Engine &engine, const Analyzer::ParameterState &parameters) {
        engine.prepare(sampleRate, blockSize);
        engine.setParameters(parameters);
    }

    std::vector<TestTone> makeTestTones(const std::vector<float> &frequenciesHz, const std::vector<float> &polarities) {
        std::vector<TestTone> tones(frequenciesHz.size());
        juce::dsp::ProcessSpec processSpec{};
        processSpec.sampleRate = sampleRate;
        processSpec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
        // Each TestTone owns one oscillator, so each one is prepared as mono
        processSpec.numChannels = 1;

        for (size_t channel = 0; channel < frequenciesHz.size(); ++channel) {
            tones[channel].oscillator.initialise([](float phase) { return std::sin(phase); });
            tones[channel].oscillator.prepare(processSpec);
            tones[channel].oscillator.setFrequency(frequenciesHz[channel], true);
            tones[channel].polarity = polarities[channel];
        }

        return tones;
    }

    void renderSineBlock(juce::AudioBuffer<float> &buffer, std::vector<TestTone> &tones, float amplitude) {
        buffer.clear();

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto *channelData = buffer.getWritePointer(channel);
            auto &tone = tones[static_cast<size_t>(channel)];

            for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex) {
                channelData[sampleIndex] = amplitude * tone.polarity * tone.oscillator.processSample(0.0f);
            }
        }
    }

    void processSineBlocks(Analyzer::Engine &engine, int channels, const std::vector<float> &frequenciesHz,
                           const std::vector<float> &polarities, int numBlocks = settleBlocks) {
        juce::AudioBuffer<float> buffer(channels, blockSize);
        auto tones = makeTestTones(frequenciesHz, polarities);

        for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex) {
            renderSineBlock(buffer, tones, testAmplitude);
            engine.processBlock(buffer);
        }
    }

    void processSilenceBlocks(Analyzer::Engine &engine, int channels, int numBlocks) {
        juce::AudioBuffer<float> buffer(channels, blockSize);
        buffer.clear();

        for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex)
            engine.processBlock(buffer);
    }

    int getStrongestBandIndex(const std::vector<float> &values) {
        const auto strongest = std::max_element(values.begin(), values.end());
        return static_cast<int>(std::distance(values.begin(), strongest));
    }

    int getNearestBandIndex(const std::vector<Analyzer::BandInfo> &bandInfo, float frequencyHz) {
        auto distanceToTarget = [frequencyHz](const Analyzer::BandInfo &band) {
            return std::abs(band.centerHz - frequencyHz);
        };

        const auto nearest = std::min_element(bandInfo.begin(), bandInfo.end(),
                                              [distanceToTarget](const Analyzer::BandInfo &lhs,
                                                                 const Analyzer::BandInfo &rhs) {
                                                  return distanceToTarget(lhs) < distanceToTarget(rhs);
                                              });

        return static_cast<int>(std::distance(bandInfo.begin(), nearest));
    }

    void requireStrongestBandNearFrequency(const Analyzer::Engine &engine, float frequencyHz, float floorDb) {
        const auto snapshot = engine.getSnapshot();
        const auto &bandInfo = snapshot.bandInfo;
        const auto &frame = snapshot.frame;
        const auto strongestBandIndex = getStrongestBandIndex(frame.peakDb);
        const auto nearestBandIndex = getNearestBandIndex(bandInfo, frequencyHz);

        REQUIRE(strongestBandIndex >= 0);
        REQUIRE(strongestBandIndex < static_cast<int>(bandInfo.size()));
        // Assert that it is at least a little loud
        REQUIRE(frame.peakDb[static_cast<size_t>(strongestBandIndex)] > floorDb + 1.0f);
        // Assert on band location instead of exact dB because filter gain and meter ballistics are still moving
        REQUIRE(std::abs(strongestBandIndex - nearestBandIndex) <= 2);
    }

    void requireHeldPeakStaysPinnedForOneSilentBlock(Analyzer::Engine &engine, int channels, float frequencyHz,
                                                     float floorDb) {
        const auto snapshotBeforeSilence = engine.getSnapshot();
        const auto &bandInfo = snapshotBeforeSilence.bandInfo;
        const auto &heldFrameBeforeSilence = snapshotBeforeSilence.frame;
        const auto strongestHoldBandIndex = getStrongestBandIndex(heldFrameBeforeSilence.holdDb);
        const auto nearestBandIndex = getNearestBandIndex(bandInfo, frequencyHz);
        const auto heldPeakBeforeSilence = heldFrameBeforeSilence.holdDb[static_cast<size_t>(strongestHoldBandIndex)];

        REQUIRE(std::abs(strongestHoldBandIndex - nearestBandIndex) <= 2);
        REQUIRE(heldPeakBeforeSilence > floorDb + 1.0f);

        processSilenceBlocks(engine, channels, 1);

        const auto heldSnapshotAfterSilence = engine.getSnapshot();
        const auto &heldFrameAfterSilence = heldSnapshotAfterSilence.frame;
        constexpr float tolerance = 0.0001f;
        REQUIRE(std::abs(heldFrameAfterSilence.holdDb[static_cast<size_t>(strongestHoldBandIndex)] - heldPeakBeforeSilence) < tolerance);
    }
}

TEST_CASE("AnalyzerEngine prepare sizes match selected band mode") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.bandMode = ParamSpec::BandMode::bands60;

    prepareEngine(engine, parameters);

    const auto snapshot = engine.getSnapshot();
    REQUIRE(snapshot.bandInfo.size() == 60);
    REQUIRE(snapshot.frame.rmsDb.size() == 60);
    REQUIRE(snapshot.frame.peakDb.size() == 60);
    REQUIRE(snapshot.frame.holdDb.size() == 60);
}

TEST_CASE("AnalyzerEngine band layout is geometrically spaced") {
    Analyzer::Engine engine;
    const auto parameters = makeDefaultParameters();

    prepareEngine(engine, parameters);

    const auto snapshot = engine.getSnapshot();
    const auto &bandInfo = snapshot.bandInfo;
    constexpr float tolerance = 0.0001f;

    REQUIRE(bandInfo.size() > 2);

    const auto expectedRatio = bandInfo[1].lowHz / bandInfo[0].lowHz;

    for (size_t bandIndex = 0; bandIndex < bandInfo.size(); ++bandIndex) {
        const auto &band = bandInfo[bandIndex];

        REQUIRE(band.lowHz < band.centerHz);
        REQUIRE(band.centerHz < band.highHz);
        REQUIRE(std::abs((band.centerHz * band.centerHz) - (band.lowHz * band.highHz)) < tolerance * band.highHz * band.highHz);

        if (bandIndex > 0) {
            const auto lowRatio = band.lowHz / bandInfo[bandIndex - 1].lowHz;
            const auto highRatio = band.highHz / bandInfo[bandIndex - 1].highHz;

            REQUIRE(std::abs(lowRatio - expectedRatio) < tolerance);
            REQUIRE(std::abs(highRatio - expectedRatio) < tolerance);
        }
    }
}

TEST_CASE("AnalyzerEngine silence stays at the configured floor") {
    Analyzer::Engine engine;
    const auto parameters = makeDefaultParameters();
    prepareEngine(engine, parameters);

    processSilenceBlocks(engine, 2, settleBlocks);

    const auto snapshot = engine.getSnapshot();
    const auto &frame = snapshot.frame;
    constexpr float tolerance = 0.001f;

    for (auto peakDb: frame.peakDb)
        REQUIRE(std::abs(peakDb - parameters.gridMinDb) < tolerance);

    for (auto rmsDb: frame.rmsDb)
        REQUIRE(std::abs(rmsDb - parameters.gridMinDb) < tolerance);
}

TEST_CASE("AnalyzerEngine summed mode tracks a low sine wave") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::summed;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    requireStrongestBandNearFrequency(engine, lowSineHz, parameters.gridMinDb);
}

TEST_CASE("AnalyzerEngine summed mode tracks a high sine wave") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::summed;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    requireStrongestBandNearFrequency(engine, highSineHz, parameters.gridMinDb);
}

TEST_CASE("AnalyzerEngine summed mode cancels opposite-phase stereo content") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::summed;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 2, {lowSineHz, lowSineHz}, {1.0f, -1.0f});

    const auto snapshot = engine.getSnapshot();
    const auto &frame = snapshot.frame;
    constexpr float tolerance = 0.001f;

    for (auto peakDb: frame.peakDb)
        REQUIRE(std::abs(peakDb - parameters.gridMinDb) < tolerance);

    for (auto rmsDb: frame.rmsDb)
        REQUIRE(std::abs(rmsDb - parameters.gridMinDb) < tolerance);
}

TEST_CASE("AnalyzerEngine stereo mode keeps low opposite-phase stereo content") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::stereo;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 2, {lowSineHz, lowSineHz}, {1.0f, -1.0f});

    requireStrongestBandNearFrequency(engine, lowSineHz, parameters.gridMinDb);
}

TEST_CASE("AnalyzerEngine stereo mode keeps high opposite-phase stereo content") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::stereo;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 2, {highSineHz, highSineHz}, {1.0f, -1.0f});

    requireStrongestBandNearFrequency(engine, highSineHz, parameters.gridMinDb);
}

TEST_CASE("AnalyzerEngine stereo mode still works with mono input") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::stereo;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    requireStrongestBandNearFrequency(engine, lowSineHz, parameters.gridMinDb);
}

TEST_CASE("AnalyzerEngine hold keeps a low sine wave pinned after it stops") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::summed;
    parameters.showHold = true;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    requireHeldPeakStaysPinnedForOneSilentBlock(engine, 1, lowSineHz, parameters.gridMinDb);
}

TEST_CASE("AnalyzerEngine hold keeps a high sine wave pinned after it stops") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::summed;
    parameters.showHold = true;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    requireHeldPeakStaysPinnedForOneSilentBlock(engine, 1, highSineHz, parameters.gridMinDb);
}

TEST_CASE("AnalyzerEngine keeps the same tone area after band mode reconfiguration") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::summed;
    parameters.bandMode = ParamSpec::BandMode::bands30;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    requireStrongestBandNearFrequency(engine, highSineHz, parameters.gridMinDb);

    parameters.bandMode = ParamSpec::BandMode::bands60;
    engine.setParameters(parameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    const auto reconfiguredSnapshot = engine.getSnapshot();
    REQUIRE(reconfiguredSnapshot.bandInfo.size() == 60);
    REQUIRE(reconfiguredSnapshot.frame.peakDb.size() == 60);
    requireStrongestBandNearFrequency(engine, highSineHz, parameters.gridMinDb);
}

TEST_CASE("AnalyzerEngine keeps working after parameter changes") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.analysisMode = ParamSpec::AnalysisMode::summed;
    parameters.bandMode = ParamSpec::BandMode::bands30;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    const auto initialSnapshot = engine.getSnapshot();
    REQUIRE(initialSnapshot.bandInfo.size() == 30);
    requireStrongestBandNearFrequency(engine, lowSineHz, parameters.gridMinDb);

    parameters.analysisMode = ParamSpec::AnalysisMode::stereo;
    parameters.bandMode = ParamSpec::BandMode::bands60;
    engine.setParameters(parameters);

    processSineBlocks(engine, 2, {highSineHz, highSineHz}, {1.0f, -1.0f});

    const auto updatedSnapshot = engine.getSnapshot();
    REQUIRE(updatedSnapshot.bandInfo.size() == 60);
    REQUIRE(updatedSnapshot.frame.peakDb.size() == 60);
    requireStrongestBandNearFrequency(engine, highSineHz, parameters.gridMinDb);
}
