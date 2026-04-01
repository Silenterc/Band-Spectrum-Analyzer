#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "dsp/core/AnalyzerEngine.h"
#include "shared/DefaultParameterValues.h"
#include "ui/analyzer/AnalyzerUiConstants.h"
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

    void prepareEngine(Analyzer::Engine &engine,
                       const TestParameters &parameters,
                       const double sampleRateToUse = sampleRate) {
        engine.prepare(sampleRateToUse, blockSize);
        engine.setParameters(parameters.engineParameters);
    }

    std::vector<TestTone> makeTestTones(const std::vector<float> &frequenciesHz,
                                        const std::vector<float> &polarities,
                                        const double sampleRateToUse = sampleRate) {
        std::vector<TestTone> tones(frequenciesHz.size());
        juce::dsp::ProcessSpec processSpec{};
        processSpec.sampleRate = sampleRateToUse;
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

    void processSineBlocks(Analyzer::Engine &engine,
                           int channels,
                           const std::vector<float> &frequenciesHz,
                           const std::vector<float> &polarities,
                           int numBlocks = settleBlocks,
                           const double sampleRateToUse = sampleRate) {
        juce::AudioBuffer<float> buffer(channels, blockSize);
        auto tones = makeTestTones(frequenciesHz, polarities, sampleRateToUse);

        for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex) {
            renderSineBlock(buffer, tones, testAmplitude);
            engine.processBlock(buffer);
        }
    }

    void processSidechainSineBlocks(Analyzer::Engine &engine, int mainChannels, int sidechainChannels,
                                    const std::vector<float> &frequenciesHz,
                                    const std::vector<float> &polarities,
                                    int numBlocks = settleBlocks,
                                    const double sampleRateToUse = sampleRate) {
        juce::AudioBuffer<float> mainBuffer(mainChannels, blockSize);
        juce::AudioBuffer<float> sidechainBuffer(sidechainChannels, blockSize);
        auto tones = makeTestTones(frequenciesHz, polarities, sampleRateToUse);
        mainBuffer.clear();

        for (int blockIndex = 0; blockIndex < numBlocks; ++blockIndex) {
            renderSineBlock(sidechainBuffer, tones, testAmplitude);
            engine.processBlock(mainBuffer, &sidechainBuffer);
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

    int getStrongestMeasurementBandIndex(const std::vector<Analyzer::BandMeasurements> &measurements) {
        const auto strongest = std::max_element(measurements.begin(), measurements.end(),
                                                [](const Analyzer::BandMeasurements &lhs,
                                                   const Analyzer::BandMeasurements &rhs) {
                                                    return lhs.peakPower < rhs.peakPower;
                                                });
        return static_cast<int>(std::distance(measurements.begin(), strongest));
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
                                        float dtSeconds = Ui::AnalyzerConstants::meterPollIntervalSeconds) {
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

    Analyzer::RenderData tickMeter(AnalyzerMeter &displayMeter,
                                   const std::vector<Analyzer::BandInfo> &bandInfo,
                                   const std::vector<Analyzer::RawTrace> &traces,
                                   const Analyzer::MeterSettings &meterSettings,
                                   const float floorDb,
                                   const float dtSeconds) {
        const auto sharedBandInfo = std::make_shared<const std::vector<Analyzer::BandInfo>>(bandInfo);
        displayMeter.tick(sharedBandInfo, traces, meterSettings, floorDb, dtSeconds);
        return displayMeter.getRenderData();
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

TEST_CASE("AnalyzerEngine clears sidechain traces when sidechain input disappears") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::sidechain, Analyzer::SignalMode::mid);

    prepareEngine(engine, parameters);
    processSidechainSineBlocks(engine, 2, 1, {lowSineHz}, {1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, lowSineHz);

    processSilenceBlocks(engine, 2, 1);

    const auto traces = engine.getTraces();
    REQUIRE(traces.size() == 1);
    constexpr float powerTolerance = 0.000001f;

    for (const auto &measurement: traces.front().measurements) {
        REQUIRE(std::abs(measurement.peakPower) < powerTolerance);
        REQUIRE(std::abs(measurement.sumPower) < powerTolerance);
        REQUIRE(measurement.numSamples == 0);
    }
}

TEST_CASE("AnalyzerEngine raw filterbank peaks on the exact band center frequency") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();

    SECTION("30 bands") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::bands30;
    }

    SECTION("45 bands") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::bands45;
    }

    SECTION("60 bands") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::bands60;
    }

    prepareEngine(engine, parameters);

    const auto bandInfo = engine.getBandInfo();
    REQUIRE(bandInfo != nullptr);
    REQUIRE(bandInfo->size() > 6);

    const std::array<size_t, 3> bandIndicesToTest{
        bandInfo->size() / 4,
        bandInfo->size() / 2,
        (bandInfo->size() * 3) / 4
    };

    for (const auto bandIndex: bandIndicesToTest) {
        DYNAMIC_SECTION("band index " << bandIndex) {
            engine.reset();

            const auto frequencyHz = (*bandInfo)[bandIndex].centerHz;
            processSineBlocks(engine, 2, {frequencyHz, frequencyHz}, {1.0f, 1.0f});

            const auto traces = engine.getTraces();
            REQUIRE(traces.size() == 1);
            REQUIRE(traces.front().measurements.size() == bandInfo->size());

            const auto strongestBandIndex = getStrongestMeasurementBandIndex(traces.front().measurements);
            REQUIRE(strongestBandIndex == static_cast<int>(bandIndex));

            const auto centerPeakPower = traces.front().measurements[bandIndex].peakPower;
            REQUIRE(centerPeakPower > 0.0f);

            if (bandIndex > 0)
                REQUIRE(centerPeakPower >= traces.front().measurements[bandIndex - 1].peakPower);

            if (bandIndex + 1 < traces.front().measurements.size())
                REQUIRE(centerPeakPower >= traces.front().measurements[bandIndex + 1].peakPower);
        }
    }
}

TEST_CASE("AnalyzerEngine 44.1 kHz 60-band centers keep consistent raw peak level within 0.5 dB") {
    constexpr double sampleRate44100 = 44100.0;
    constexpr float floorDb = -200.0f;
    constexpr float allowedSpreadDb = 0.5f;
    constexpr int consistencySettleBlocks = 96;

    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.engineParameters.bandMode = Analyzer::BandMode::bands60;

    prepareEngine(engine, parameters, sampleRate44100);

    const auto bandInfo = engine.getBandInfo();
    REQUIRE(bandInfo != nullptr);
    REQUIRE(bandInfo->size() == 60);

    std::vector<float> centerPeakDb;
    centerPeakDb.reserve(bandInfo->size());

    for (size_t bandIndex = 0; bandIndex < bandInfo->size(); ++bandIndex) {
        engine.reset();

        const auto frequencyHz = (*bandInfo)[bandIndex].centerHz;
        processSineBlocks(engine, 2, {frequencyHz, frequencyHz}, {1.0f, 1.0f}, consistencySettleBlocks, sampleRate44100);

        const auto traces = engine.getTraces();
        REQUIRE(traces.size() == 1);
        REQUIRE(traces.front().measurements.size() == bandInfo->size());

        const auto strongestBandIndex = getStrongestMeasurementBandIndex(traces.front().measurements);
        CAPTURE(bandIndex, frequencyHz, strongestBandIndex);
        REQUIRE(strongestBandIndex == static_cast<int>(bandIndex));

        const auto &measurement = traces.front().measurements[bandIndex];
        REQUIRE(measurement.numSamples > 0);

        centerPeakDb.push_back(juce::Decibels::gainToDecibels(std::sqrt(measurement.peakPower), floorDb));
    }

    const auto [minPeakIt, maxPeakIt] = std::minmax_element(centerPeakDb.begin(), centerPeakDb.end());
    const auto peakSpreadDb = *maxPeakIt - *minPeakIt;
    const auto minPeakIndex = static_cast<size_t>(std::distance(centerPeakDb.begin(), minPeakIt));
    const auto maxPeakIndex = static_cast<size_t>(std::distance(centerPeakDb.begin(), maxPeakIt));

    CAPTURE(peakSpreadDb,
            minPeakIndex,
            maxPeakIndex,
            (*bandInfo)[minPeakIndex].centerHz,
            (*bandInfo)[maxPeakIndex].centerHz);

    REQUIRE(peakSpreadDb <= allowedSpreadDb);
}

TEST_CASE("AnalyzerMeter RMS release stays aligned with peak decay") {
    AnalyzerMeter displayMeter;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showRms = true;
    meterSettings.showPeak = true;
    meterSettings.showHold = false;

    constexpr float floorDb = -80.0f;
    constexpr float dtSeconds = 0.1f;
    constexpr float toleranceDb = 0.01f;

    const std::vector<Analyzer::BandInfo> bandInfo{
        {.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f}
    };

    Analyzer::RawTrace activeTrace;
    activeTrace.kind = Analyzer::TraceKind::slot1;
    activeTrace.measurements.resize(1);
    activeTrace.measurements[0].peakPower = 1.0f;
    activeTrace.measurements[0].sumPower = 1024.0;
    activeTrace.measurements[0].numSamples = 1024;

    const auto activeData = tickMeter(displayMeter, bandInfo, {activeTrace}, meterSettings, floorDb, dtSeconds);
    REQUIRE(activeData.traces.size() == 1);
    REQUIRE(std::abs(activeData.traces.front().frame.rmsDb[0] - 0.0f) < toleranceDb);
    REQUIRE(std::abs(activeData.traces.front().frame.peakDb[0] - 0.0f) < toleranceDb);

    Analyzer::RawTrace silentTrace = activeTrace;
    silentTrace.measurements[0].peakPower = 0.0f;
    silentTrace.measurements[0].sumPower = 0.0;

    const auto firstSilentData = tickMeter(displayMeter, bandInfo, {silentTrace}, meterSettings, floorDb, dtSeconds);
    REQUIRE(std::abs(firstSilentData.traces.front().frame.rmsDb[0] + 1.5f) < toleranceDb);
    REQUIRE(std::abs(firstSilentData.traces.front().frame.peakDb[0] + 1.5f) < toleranceDb);

    const auto secondSilentData = tickMeter(displayMeter, bandInfo, {silentTrace}, meterSettings, floorDb, dtSeconds);
    REQUIRE(std::abs(secondSilentData.traces.front().frame.rmsDb[0] + 3.0f) < toleranceDb);
    REQUIRE(std::abs(secondSilentData.traces.front().frame.peakDb[0] + 3.0f) < toleranceDb);
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
