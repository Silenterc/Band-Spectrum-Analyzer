#include <algorithm>
#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "dsp/AnalyzerEngine.h"
#include "dsp/AnalyzerConstants.h"
#include "shared/DefaultParameterValues.h"
#include "ui/analyzer/helpers/AnalyzerMeter.h"

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

    struct TestParameters {
        Analyzer::EngineParameterState engineParameters;
        bool showRms = true;
        bool showPeak = true;
        bool showHold = false;
        float holdMs = Defaults::holdMs;
        float gridMinDb = Defaults::gridMinDb;
        float gridMaxDb = Defaults::gridMaxDb;
        float gridStepDb = Defaults::gridStepDb;
    };

    TestParameters makeDefaultParameters() {
        TestParameters parameters;
        parameters.engineParameters.bandMode = Analyzer::BandMode::bands45;
        parameters.engineParameters.signalSlots[0].enabled = true;
        parameters.engineParameters.signalSlots[0].source = Analyzer::SignalSource::main;
        parameters.engineParameters.signalSlots[0].mode = Analyzer::SignalMode::mid;
        return parameters;
    }

    void setPrimarySignal(TestParameters &parameters,
                          const Analyzer::SignalSource source,
                          const Analyzer::SignalMode mode) {
        parameters.engineParameters.signalSlots[0].enabled = true;
        parameters.engineParameters.signalSlots[0].source = source;
        parameters.engineParameters.signalSlots[0].mode = mode;
    }

    Analyzer::MeterSettings makeMeterSettings(const TestParameters &parameters) {
        Analyzer::MeterSettings meterSettings;
        meterSettings.showRms = parameters.showRms;
        meterSettings.showPeak = parameters.showPeak;
        meterSettings.showHold = parameters.showHold;
        meterSettings.holdMs = parameters.holdMs;
        return meterSettings;
    }

    void prepareEngine(Analyzer::Engine &engine, const TestParameters &parameters) {
        engine.prepare(sampleRate, blockSize);
        engine.setParameters(parameters.engineParameters);
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

    Analyzer::RenderData buildMeterData(AnalyzerMeter &displayMeter, const Analyzer::Engine &engine,
                                        const TestParameters &parameters,
                                        float dtSeconds = Analyzer::Constants::meterPollIntervalSeconds) {
        displayMeter.tick(engine.getBandInfo(), engine.getTraces(), makeMeterSettings(parameters), parameters.gridMinDb, dtSeconds);
        return displayMeter.getRenderData();
    }

    void requireStrongestBandNearFrequency(AnalyzerMeter &displayMeter, const Analyzer::Engine &engine,
                                           const TestParameters &parameters, float frequencyHz) {
        const auto meterData = buildMeterData(displayMeter, engine, parameters);
        const auto &bandInfo = meterData.bandInfo;
        const auto &frame = meterData.traces.front().frame;
        const auto strongestBandIndex = getStrongestBandIndex(frame.peakDb);
        const auto nearestBandIndex = getNearestBandIndex(bandInfo, frequencyHz);

        REQUIRE(strongestBandIndex >= 0);
        REQUIRE(strongestBandIndex < static_cast<int>(bandInfo.size()));
        // Assert that it is at least a little loud
        REQUIRE(frame.peakDb[static_cast<size_t>(strongestBandIndex)] > parameters.gridMinDb + 1.0f);
        // Assert on band location instead of exact dB because filter gain and meter ballistics are still moving
        REQUIRE(std::abs(strongestBandIndex - nearestBandIndex) <= 2);
    }

    void requireHeldPeakStaysPinnedForOneSilentBlock(AnalyzerMeter &displayMeter, Analyzer::Engine &engine,
                                                     const TestParameters &parameters, int channels,
                                                     float frequencyHz) {
        const auto meterDataBeforeSilence = buildMeterData(displayMeter, engine, parameters);
        const auto &bandInfo = meterDataBeforeSilence.bandInfo;
        const auto &heldFrameBeforeSilence = meterDataBeforeSilence.traces.front().frame;
        const auto strongestHoldBandIndex = getStrongestBandIndex(heldFrameBeforeSilence.holdDb);
        const auto nearestBandIndex = getNearestBandIndex(bandInfo, frequencyHz);
        const auto heldPeakBeforeSilence = heldFrameBeforeSilence.holdDb[static_cast<size_t>(strongestHoldBandIndex)];

        REQUIRE(std::abs(strongestHoldBandIndex - nearestBandIndex) <= 2);
        REQUIRE(heldPeakBeforeSilence > parameters.gridMinDb + 1.0f);

        processSilenceBlocks(engine, channels, 1);

        const auto meterDataAfterSilence = buildMeterData(displayMeter, engine, parameters,
                                                              static_cast<float>(blockSize) / static_cast<float>(sampleRate));
        const auto &heldFrameAfterSilence = meterDataAfterSilence.traces.front().frame;
        constexpr float tolerance = 0.0001f;
        REQUIRE(std::abs(heldFrameAfterSilence.holdDb[static_cast<size_t>(strongestHoldBandIndex)] - heldPeakBeforeSilence) < tolerance);
    }
}

TEST_CASE("AnalyzerEngine prepare sizes match selected band mode") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    parameters.engineParameters.bandMode = Analyzer::BandMode::bands60;

    prepareEngine(engine, parameters);

    const auto meterData = buildMeterData(displayMeter, engine, parameters);
    REQUIRE(meterData.bandInfo.size() == 60);
    REQUIRE(meterData.traces.front().frame.rmsDb.size() == 60);
    REQUIRE(meterData.traces.front().frame.peakDb.size() == 60);
    REQUIRE(meterData.traces.front().frame.holdDb.size() == 60);
}

TEST_CASE("AnalyzerEngine band layout is geometrically spaced") {
    Analyzer::Engine engine;
    const auto parameters = makeDefaultParameters();

    prepareEngine(engine, parameters);

    const auto bandInfo = engine.getBandInfo();
    constexpr float tolerance = 0.0001f;

    REQUIRE(bandInfo->size() > 2);

    const auto expectedRatio = (*bandInfo)[1].lowHz / (*bandInfo)[0].lowHz;

    for (size_t bandIndex = 0; bandIndex < bandInfo->size(); ++bandIndex) {
        const auto &band = (*bandInfo)[bandIndex];

        REQUIRE(band.lowHz < band.centerHz);
        REQUIRE(band.centerHz < band.highHz);
        REQUIRE(std::abs((band.centerHz * band.centerHz) - (band.lowHz * band.highHz)) < tolerance * band.highHz * band.highHz);

        if (bandIndex > 0) {
            const auto lowRatio = band.lowHz / (*bandInfo)[bandIndex - 1].lowHz;
            const auto highRatio = band.highHz / (*bandInfo)[bandIndex - 1].highHz;

            REQUIRE(std::abs(lowRatio - expectedRatio) < tolerance);
            REQUIRE(std::abs(highRatio - expectedRatio) < tolerance);
        }
    }
}

TEST_CASE("AnalyzerEngine silence stays at the configured floor") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    const auto parameters = makeDefaultParameters();
    prepareEngine(engine, parameters);

    processSilenceBlocks(engine, 2, settleBlocks);

    const auto meterData = buildMeterData(displayMeter, engine, parameters);
    const auto &frame = meterData.traces.front().frame;
    constexpr float tolerance = 0.001f;

    for (auto peakDb: frame.peakDb)
        REQUIRE(std::abs(peakDb - parameters.gridMinDb) < tolerance);

    for (auto rmsDb: frame.rmsDb)
        REQUIRE(std::abs(rmsDb - parameters.gridMinDb) < tolerance);
}

TEST_CASE("AnalyzerEngine summed mode tracks a low sine wave") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, lowSineHz);
}

TEST_CASE("AnalyzerEngine summed mode tracks a high sine wave") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, highSineHz);
}

TEST_CASE("AnalyzerEngine summed mode cancels opposite-phase stereo content") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 2, {lowSineHz, lowSineHz}, {1.0f, -1.0f});

    const auto meterData = buildMeterData(displayMeter, engine, parameters);
    const auto &frame = meterData.traces.front().frame;
    constexpr float tolerance = 0.001f;

    for (auto peakDb: frame.peakDb)
        REQUIRE(std::abs(peakDb - parameters.gridMinDb) < tolerance);

    for (auto rmsDb: frame.rmsDb)
        REQUIRE(std::abs(rmsDb - parameters.gridMinDb) < tolerance);
}

TEST_CASE("AnalyzerEngine stereo mode keeps low opposite-phase stereo content") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::stereo);

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 2, {lowSineHz, lowSineHz}, {1.0f, -1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, lowSineHz);
}

TEST_CASE("AnalyzerEngine stereo mode keeps high opposite-phase stereo content") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::stereo);

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 2, {highSineHz, highSineHz}, {1.0f, -1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, highSineHz);
}

TEST_CASE("AnalyzerEngine stereo mode still works with mono input") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::stereo);

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, lowSineHz);
}

TEST_CASE("AnalyzerEngine hold keeps a low sine wave pinned after it stops") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);
    parameters.showHold = true;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    requireHeldPeakStaysPinnedForOneSilentBlock(displayMeter, engine, parameters, 1, lowSineHz);
}

TEST_CASE("AnalyzerEngine hold keeps a high sine wave pinned after it stops") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);
    parameters.showHold = true;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    requireHeldPeakStaysPinnedForOneSilentBlock(displayMeter, engine, parameters, 1, highSineHz);
}

TEST_CASE("AnalyzerEngine keeps the same tone area after band mode reconfiguration") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);
    parameters.engineParameters.bandMode = Analyzer::BandMode::bands30;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, highSineHz);

    parameters.engineParameters.bandMode = Analyzer::BandMode::bands60;
    engine.setParameters(parameters.engineParameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    const auto reconfiguredSnapshot = buildMeterData(displayMeter, engine, parameters);
    REQUIRE(reconfiguredSnapshot.bandInfo.size() == 60);
    REQUIRE(reconfiguredSnapshot.traces.front().frame.peakDb.size() == 60);
    requireStrongestBandNearFrequency(displayMeter, engine, parameters, highSineHz);
}

TEST_CASE("AnalyzerEngine keeps working after parameter changes") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);
    parameters.engineParameters.bandMode = Analyzer::BandMode::bands30;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    const auto initialSnapshot = buildMeterData(displayMeter, engine, parameters);
    REQUIRE(initialSnapshot.bandInfo.size() == 30);
    requireStrongestBandNearFrequency(displayMeter, engine, parameters, lowSineHz);

    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::stereo);
    parameters.engineParameters.bandMode = Analyzer::BandMode::bands60;
    engine.setParameters(parameters.engineParameters);

    processSineBlocks(engine, 2, {highSineHz, highSineHz}, {1.0f, -1.0f});

    const auto updatedSnapshot = buildMeterData(displayMeter, engine, parameters);
    REQUIRE(updatedSnapshot.bandInfo.size() == 60);
    REQUIRE(updatedSnapshot.traces.front().frame.peakDb.size() == 60);
    requireStrongestBandNearFrequency(displayMeter, engine, parameters, highSineHz);
}
