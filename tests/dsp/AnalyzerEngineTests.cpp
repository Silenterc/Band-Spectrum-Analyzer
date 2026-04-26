#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "dsp/core/AnalyzerConstants.h"
#include "dsp/core/AnalyzerEngine.h"
#include "display/analyzer/logic/AnalyzerMeter.h"
#include "shared/DefaultParameterValues.h"
#include "ui/analyzer/plot/state/AnalyzerUiConstants.h"

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
        float rmsWindowMs = Defaults::rmsWindowMs;
        float gridMinDb = Defaults::gridMinDb;
        float gridMaxDb = Defaults::gridMaxDb;
        float gridStepDb = Defaults::gridStepDb;
    };

    TestParameters makeDefaultParameters() {
        TestParameters parameters;
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveSixth;
        parameters.engineParameters.signalSlots[0].enabled = true;
        parameters.engineParameters.signalSlots[0].source = Analyzer::SignalSource::main;
        parameters.engineParameters.signalSlots[0].mode = Analyzer::SignalMode::mid;
        return parameters;
    }

    int getBandsPerOctave(const Analyzer::BandMode bandMode) {
        switch (bandMode) {
            case Analyzer::BandMode::octaveThird:
                return 3;
            case Analyzer::BandMode::octaveQuarter:
                return 4;
            case Analyzer::BandMode::octaveSixth:
                return 6;
            case Analyzer::BandMode::octaveTwelfth:
                return 12;
        }

        return 6;
    }

    double getMaxAnalysisFrequencyHz(const double sampleRateToUse) {
        return std::max(sampleRateToUse * 0.5 * static_cast<double>(Analyzer::Constants::maxAnalysisFractionOfNyquist),
                        static_cast<double>(Analyzer::Constants::minFrequencyHz * 2.0f));
    }

    size_t getExpectedBandCount(const double sampleRateToUse, const Analyzer::BandMode bandMode) {
        const auto centerRatio = std::pow(2.0, 1.0 / static_cast<double>(getBandsPerOctave(bandMode)));
        const auto edgeRatio = std::sqrt(centerRatio);
        auto centerHz = Analyzer::Constants::equalTemperedAnchorHz;
        const auto maxAnalysisFrequencyHz = getMaxAnalysisFrequencyHz(sampleRateToUse);

        while (centerHz / edgeRatio < static_cast<double>(Analyzer::Constants::minFrequencyHz))
            centerHz *= centerRatio;

        while (true) {
            const auto previousCenterHz = centerHz / centerRatio;
            const auto previousLowHz = previousCenterHz / edgeRatio;
            if (previousLowHz < static_cast<double>(Analyzer::Constants::minFrequencyHz))
                break;

            centerHz = previousCenterHz;
        }

        size_t bandCount = 0;
        while (true) {
            const auto highHz = centerHz * edgeRatio;
            if (highHz > maxAnalysisFrequencyHz)
                break;

            ++bandCount;
            centerHz *= centerRatio;
        }

        return bandCount;
    }

    double getExpectedFirstCenterHz(const Analyzer::BandMode bandMode) {
        const auto centerRatio = std::pow(2.0, 1.0 / static_cast<double>(getBandsPerOctave(bandMode)));
        const auto edgeRatio = std::sqrt(centerRatio);
        auto centerHz = Analyzer::Constants::equalTemperedAnchorHz;

        while (centerHz / edgeRatio < static_cast<double>(Analyzer::Constants::minFrequencyHz))
            centerHz *= centerRatio;

        while (true) {
            const auto previousCenterHz = centerHz / centerRatio;
            const auto previousLowHz = previousCenterHz / edgeRatio;
            if (previousLowHz < static_cast<double>(Analyzer::Constants::minFrequencyHz))
                break;

            centerHz = previousCenterHz;
        }

        return centerHz;
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
        meterSettings.rmsWindowMs = parameters.rmsWindowMs;
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

    Analyzer::MeterData buildMeterData(AnalyzerMeter &displayMeter, const Analyzer::Engine &engine,
                                       const TestParameters &parameters,
                                       float dtSeconds = Ui::AnalyzerConstants::meterPollIntervalSeconds) {
        const auto publishedTraces = engine.readPublishedTraces();
        displayMeter.tick(engine.getBandInfo(),
                          publishedTraces.getTraces(),
                          publishedTraces.hasUpdate,
                          false,
                          publishedTraces.hopDurationSeconds,
                          makeMeterSettings(parameters),
                          parameters.gridMinDb,
                          dtSeconds);
        return displayMeter.getMeterData();
    }

    void requireStrongestBandNearFrequency(AnalyzerMeter &displayMeter, const Analyzer::Engine &engine,
                                           const TestParameters &parameters, float frequencyHz) {
        const auto meterData = buildMeterData(displayMeter, engine, parameters);
        REQUIRE(meterData.bandInfo != nullptr);
        const auto &bandInfo = *meterData.bandInfo;
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

    Analyzer::MeterData tickMeter(AnalyzerMeter &displayMeter,
                                  const std::vector<Analyzer::BandInfo> &bandInfo,
                                  const std::vector<Analyzer::RawTrace> &traces,
                                  const bool hasNewData,
                                  const bool advanceSilentRms,
                                  const float hopDurationSeconds,
                                  const Analyzer::MeterSettings &meterSettings,
                                  const float floorDb,
                                  const float dtSeconds) {
        const auto sharedBandInfo = std::make_shared<const std::vector<Analyzer::BandInfo>>(bandInfo);
        displayMeter.tick(sharedBandInfo,
                          traces,
                          hasNewData,
                          advanceSilentRms,
                          hopDurationSeconds,
                          meterSettings,
                          floorDb,
                          dtSeconds);
        return displayMeter.getMeterData();
    }
}

TEST_CASE("AnalyzerEngine prepare sizes match selected band mode") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();

    SECTION("1/3 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveThird;
    }

    SECTION("1/4 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveQuarter;
    }

    SECTION("1/6 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveSixth;
    }

    SECTION("1/12 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveTwelfth;
    }

    prepareEngine(engine, parameters);

    const auto meterData = buildMeterData(displayMeter, engine, parameters);
    const auto expectedBandCount = getExpectedBandCount(sampleRate, parameters.engineParameters.bandMode);
    REQUIRE(meterData.bandInfo != nullptr);
    REQUIRE(meterData.bandInfo->size() == expectedBandCount);
    REQUIRE(meterData.traces.front().frame.rmsDb.size() == expectedBandCount);
    REQUIRE(meterData.traces.front().frame.peakDb.size() == expectedBandCount);
}

TEST_CASE("AnalyzerEngine band layout is geometrically spaced") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();

    SECTION("1/3 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveThird;
    }

    SECTION("1/4 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveQuarter;
    }

    SECTION("1/6 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveSixth;
    }

    SECTION("1/12 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveTwelfth;
    }

    prepareEngine(engine, parameters);

    const auto bandInfo = engine.getBandInfo();
    constexpr float tolerance = 0.0001f;
    const auto expectedRatio = std::pow(2.0f, 1.0f / static_cast<float>(getBandsPerOctave(parameters.engineParameters.bandMode)));

    REQUIRE(bandInfo->size() > 2);
    REQUIRE((*bandInfo).front().lowHz >= Analyzer::Constants::minFrequencyHz);
    REQUIRE((*bandInfo).back().highHz <= getMaxAnalysisFrequencyHz(sampleRate) + tolerance);
    REQUIRE(std::abs((*bandInfo).front().centerHz
                     - static_cast<float>(getExpectedFirstCenterHz(parameters.engineParameters.bandMode))) < tolerance);

    for (size_t bandIndex = 0; bandIndex < bandInfo->size(); ++bandIndex) {
        const auto &band = (*bandInfo)[bandIndex];

        REQUIRE(band.lowHz < band.centerHz);
        REQUIRE(band.centerHz < band.highHz);
        REQUIRE(std::abs((band.centerHz * band.centerHz) - (band.lowHz * band.highHz)) < tolerance * band.highHz * band.highHz);

        if (bandIndex > 0) {
            const auto lowRatio = band.lowHz / (*bandInfo)[bandIndex - 1].lowHz;
            const auto centerRatio = band.centerHz / (*bandInfo)[bandIndex - 1].centerHz;
            const auto highRatio = band.highHz / (*bandInfo)[bandIndex - 1].highHz;

            REQUIRE(std::abs(lowRatio - expectedRatio) < tolerance);
            REQUIRE(std::abs(centerRatio - expectedRatio) < tolerance);
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

TEST_CASE("AnalyzerEngine clears sidechain traces after the silent hold flushes analyzer state") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::sidechain, Analyzer::SignalMode::mid);

    prepareEngine(engine, parameters);
    processSidechainSineBlocks(engine, 2, 1, {lowSineHz}, {1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, lowSineHz);

    processSilenceBlocks(engine, 2, 16);

    const auto &traces = engine.readPublishedTraces().getTraces();
    REQUIRE(traces.size() == 1);
    constexpr float powerTolerance = 0.00001f;

    for (const auto &measurement: traces.front().measurements) {
        REQUIRE(std::abs(measurement.peakPower) < powerTolerance);
        REQUIRE(std::abs(measurement.rmsHopSumPower) < powerTolerance);
        REQUIRE(measurement.rmsHopNumSamples == 0);
    }
}

TEST_CASE("AnalyzerEngine raw filterbank peaks on the exact band center frequency") {
    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();

    SECTION("1/3 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveThird;
    }

    SECTION("1/4 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveQuarter;
    }

    SECTION("1/6 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveSixth;
    }

    SECTION("1/12 octave") {
        parameters.engineParameters.bandMode = Analyzer::BandMode::octaveTwelfth;
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

            const auto &traces = engine.readPublishedTraces().getTraces();
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

TEST_CASE("AnalyzerEngine 44.1 kHz 1/6-octave centers keep consistent raw peak level within 0.5 dB") {
    constexpr double sampleRate44100 = 44100.0;
    constexpr float floorDb = -200.0f;
    constexpr float allowedSpreadDb = 0.5f;
    constexpr int consistencySettleBlocks = 96;

    Analyzer::Engine engine;
    auto parameters = makeDefaultParameters();
    parameters.engineParameters.bandMode = Analyzer::BandMode::octaveSixth;

    prepareEngine(engine, parameters, sampleRate44100);

    const auto bandInfo = engine.getBandInfo();
    REQUIRE(bandInfo != nullptr);
    REQUIRE(bandInfo->size() == getExpectedBandCount(sampleRate44100, parameters.engineParameters.bandMode));

    std::vector<float> centerPeakDb;
    centerPeakDb.reserve(bandInfo->size());

    for (size_t bandIndex = 0; bandIndex < bandInfo->size(); ++bandIndex) {
        engine.reset();

        const auto frequencyHz = (*bandInfo)[bandIndex].centerHz;
        processSineBlocks(engine, 2, {frequencyHz, frequencyHz}, {1.0f, 1.0f}, consistencySettleBlocks, sampleRate44100);

        const auto &traces = engine.readPublishedTraces().getTraces();
        REQUIRE(traces.size() == 1);
        REQUIRE(traces.front().measurements.size() == bandInfo->size());

        const auto strongestBandIndex = getStrongestMeasurementBandIndex(traces.front().measurements);
        CAPTURE(bandIndex, frequencyHz, strongestBandIndex);
        REQUIRE(strongestBandIndex == static_cast<int>(bandIndex));

        const auto &measurement = traces.front().measurements[bandIndex];
        REQUIRE(measurement.rmsHopNumSamples > 0);

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

TEST_CASE("AnalyzerMeter RMS follows fresh hop updates instead of display ticks") {
    AnalyzerMeter displayMeter;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showRms = true;
    meterSettings.showPeak = true;
    meterSettings.showHold = false;
    meterSettings.rmsWindowMs = 180.0f;

    constexpr float floorDb = -80.0f;
    constexpr float dtSeconds = 0.016f;
    constexpr float hopDurationSeconds = 0.09f;
    constexpr float toleranceDb = 0.01f;

    const std::vector<Analyzer::BandInfo> bandInfo{
        {.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f}
    };

    Analyzer::RawTrace activeTrace;
    activeTrace.kind = Analyzer::TraceKind::slot1;
    activeTrace.measurements.resize(1);
    activeTrace.measurements[0].peakPower = 1.0f;
    activeTrace.measurements[0].rmsHopSumPower = 1024.0;
    activeTrace.measurements[0].rmsHopNumSamples = 1024;

    const auto activeData = tickMeter(displayMeter,
                                      bandInfo,
                                      {activeTrace},
                                      true,
                                      false,
                                      hopDurationSeconds,
                                      meterSettings,
                                      floorDb,
                                      dtSeconds);
    REQUIRE(activeData.traces.size() == 1);
    REQUIRE(std::abs(activeData.traces.front().frame.rmsDb[0] - 0.0f) < toleranceDb);
    REQUIRE(std::abs(activeData.traces.front().frame.peakDb[0] - 0.0f) < toleranceDb);

    const auto noUpdateData = tickMeter(displayMeter,
                                        bandInfo,
                                        {activeTrace},
                                        false,
                                        false,
                                        hopDurationSeconds,
                                        meterSettings,
                                        floorDb,
                                        dtSeconds);
    REQUIRE(std::abs(noUpdateData.traces.front().frame.rmsDb[0] - 0.0f) < toleranceDb);

    Analyzer::MeterData firstSilentData;
    constexpr int ticksPerSilentHop = 6;
    for (int tickIndex = 0; tickIndex < ticksPerSilentHop; ++tickIndex) {
        firstSilentData = tickMeter(displayMeter,
                                    bandInfo,
                                    {activeTrace},
                                    false,
                                    true,
                                    hopDurationSeconds,
                                    meterSettings,
                                    floorDb,
                                    dtSeconds);
    }
    REQUIRE(std::abs(firstSilentData.traces.front().frame.rmsDb[0] + 3.0103f) < 0.05f);
    REQUIRE(std::abs(firstSilentData.traces.front().frame.peakDb[0]) < toleranceDb);

    Analyzer::MeterData secondSilentData;
    for (int tickIndex = 0; tickIndex < ticksPerSilentHop; ++tickIndex) {
        secondSilentData = tickMeter(displayMeter,
                                     bandInfo,
                                     {activeTrace},
                                     false,
                                     true,
                                     hopDurationSeconds,
                                     meterSettings,
                                     floorDb,
                                     dtSeconds);
    }
    REQUIRE(std::abs(secondSilentData.traces.front().frame.rmsDb[0] - floorDb) < toleranceDb);
}

TEST_CASE("AnalyzerMeter discards peak state internally once it settles below the display floor") {
    AnalyzerMeter displayMeter;
    Analyzer::MeterSettings meterSettings;
    meterSettings.showRms = true;
    meterSettings.showPeak = true;

    constexpr float floorDb = -80.0f;
    constexpr float dtSeconds = 0.016f;
    constexpr float hopDurationSeconds = 0.09f;
    constexpr float nearFloorDb = -79.95f;
    constexpr float lowerFloorDb = -100.0f;
    constexpr float toleranceDb = 0.001f;

    const std::vector<Analyzer::BandInfo> bandInfo{
        {.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f}
    };

    Analyzer::RawTrace trace;
    trace.kind = Analyzer::TraceKind::slot1;
    trace.measurements.resize(1);
    const auto nearFloorGain = juce::Decibels::decibelsToGain(nearFloorDb);
    trace.measurements[0].peakPower = nearFloorGain * nearFloorGain;
    trace.measurements[0].rmsHopSumPower = static_cast<double>(trace.measurements[0].peakPower) * 1024.0;
    trace.measurements[0].rmsHopNumSamples = 1024;

    const auto meterData = tickMeter(displayMeter,
                                     bandInfo,
                                     {trace},
                                     true,
                                     false,
                                     hopDurationSeconds,
                                     meterSettings,
                                     floorDb,
                                     dtSeconds);

    REQUIRE(std::abs(meterData.traces.front().frame.peakDb[0] - floorDb) < toleranceDb);
    REQUIRE(displayMeter.isSettledAtFloor(floorDb));

    const auto loweredFloorData = tickMeter(displayMeter,
                                            bandInfo,
                                            {trace},
                                            false,
                                            false,
                                            hopDurationSeconds,
                                            meterSettings,
                                            lowerFloorDb,
                                            dtSeconds);

    REQUIRE(std::abs(loweredFloorData.traces.front().frame.peakDb[0] - lowerFloorDb) < toleranceDb);
    REQUIRE(std::abs(loweredFloorData.traces.front().frame.rmsDb[0] - nearFloorDb) < toleranceDb);
}

TEST_CASE("AnalyzerMeter peak decay parameter changes peak falloff speed") {
    AnalyzerMeter slowDecayMeter;
    AnalyzerMeter fastDecayMeter;
    Analyzer::MeterSettings slowDecaySettings;
    slowDecaySettings.showRms = true;
    slowDecaySettings.showPeak = true;
    slowDecaySettings.peakDecayDbPerSecond = 5.0f;

    Analyzer::MeterSettings fastDecaySettings = slowDecaySettings;
    fastDecaySettings.peakDecayDbPerSecond = 20.0f;

    constexpr float floorDb = -80.0f;
    constexpr float dtSeconds = 0.1f;
    constexpr float hopDurationSeconds = 0.09f;
    constexpr float toleranceDb = 0.001f;

    const std::vector<Analyzer::BandInfo> bandInfo{
        {.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f}
    };

    Analyzer::RawTrace activeTrace;
    activeTrace.kind = Analyzer::TraceKind::slot1;
    activeTrace.measurements.resize(1);
    activeTrace.measurements[0].peakPower = 1.0f;
    activeTrace.measurements[0].rmsHopSumPower = 1024.0;
    activeTrace.measurements[0].rmsHopNumSamples = 1024;

    Analyzer::RawTrace quieterTrace = activeTrace;
    const auto quieterPeakGain = juce::Decibels::decibelsToGain(-40.0f);
    quieterTrace.measurements[0].peakPower = quieterPeakGain * quieterPeakGain;

    tickMeter(slowDecayMeter,
              bandInfo,
              {activeTrace},
              true,
              false,
              hopDurationSeconds,
              slowDecaySettings,
              floorDb,
              dtSeconds);
    tickMeter(fastDecayMeter,
              bandInfo,
              {activeTrace},
              true,
              false,
              hopDurationSeconds,
              fastDecaySettings,
              floorDb,
              dtSeconds);

    const auto slowDecayData = tickMeter(slowDecayMeter,
                                         bandInfo,
                                         {quieterTrace},
                                         true,
                                         false,
                                         hopDurationSeconds,
                                         slowDecaySettings,
                                         floorDb,
                                         dtSeconds);
    const auto fastDecayData = tickMeter(fastDecayMeter,
                                         bandInfo,
                                         {quieterTrace},
                                         true,
                                         false,
                                         hopDurationSeconds,
                                         fastDecaySettings,
                                         floorDb,
                                         dtSeconds);

    REQUIRE(slowDecayData.traces.front().frame.peakDb[0] == Catch::Approx(-0.5f).margin(toleranceDb));
    REQUIRE(fastDecayData.traces.front().frame.peakDb[0] == Catch::Approx(-2.0f).margin(toleranceDb));
    REQUIRE(fastDecayData.traces.front().frame.peakDb[0] < slowDecayData.traces.front().frame.peakDb[0]);
}

TEST_CASE("AnalyzerMeter RMS window parameter changes averaging time") {
    AnalyzerMeter shortWindowMeter;
    AnalyzerMeter longWindowMeter;
    Analyzer::MeterSettings shortWindowSettings;
    shortWindowSettings.showRms = true;
    shortWindowSettings.showPeak = true;
    shortWindowSettings.rmsWindowMs = 90.0f;

    Analyzer::MeterSettings longWindowSettings = shortWindowSettings;
    longWindowSettings.rmsWindowMs = 180.0f;

    constexpr float floorDb = -80.0f;
    constexpr float dtSeconds = 0.016f;
    constexpr float hopDurationSeconds = 0.09f;

    const std::vector<Analyzer::BandInfo> bandInfo{
        {.lowHz = 80.0f, .centerHz = 100.0f, .highHz = 125.0f}
    };

    Analyzer::RawTrace activeTrace;
    activeTrace.kind = Analyzer::TraceKind::slot1;
    activeTrace.measurements.resize(1);
    activeTrace.measurements[0].peakPower = 1.0f;
    activeTrace.measurements[0].rmsHopSumPower = 1024.0;
    activeTrace.measurements[0].rmsHopNumSamples = 1024;

    tickMeter(shortWindowMeter,
              bandInfo,
              {activeTrace},
              true,
              false,
              hopDurationSeconds,
              shortWindowSettings,
              floorDb,
              dtSeconds);
    tickMeter(longWindowMeter,
              bandInfo,
              {activeTrace},
              true,
              false,
              hopDurationSeconds,
              longWindowSettings,
              floorDb,
              dtSeconds);

    Analyzer::MeterData shortWindowSilentData;
    Analyzer::MeterData longWindowSilentData;
    constexpr int ticksPerSilentHop = 6;
    for (int tickIndex = 0; tickIndex < ticksPerSilentHop; ++tickIndex) {
        shortWindowSilentData = tickMeter(shortWindowMeter,
                                          bandInfo,
                                          {activeTrace},
                                          false,
                                          true,
                                          hopDurationSeconds,
                                          shortWindowSettings,
                                          floorDb,
                                          dtSeconds);
        longWindowSilentData = tickMeter(longWindowMeter,
                                         bandInfo,
                                         {activeTrace},
                                         false,
                                         true,
                                         hopDurationSeconds,
                                         longWindowSettings,
                                         floorDb,
                                         dtSeconds);
    }

    REQUIRE(std::abs(shortWindowSilentData.traces.front().frame.rmsDb[0] - floorDb) < 0.01f);
    REQUIRE(std::abs(longWindowSilentData.traces.front().frame.rmsDb[0] + 3.0103f) < 0.05f);
}

TEST_CASE("AnalyzerEngine keeps the same tone area after band mode reconfiguration") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);
    parameters.engineParameters.bandMode = Analyzer::BandMode::octaveThird;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    requireStrongestBandNearFrequency(displayMeter, engine, parameters, highSineHz);

    parameters.engineParameters.bandMode = Analyzer::BandMode::octaveSixth;
    engine.setParameters(parameters.engineParameters);
    processSineBlocks(engine, 1, {highSineHz}, {1.0f});

    const auto reconfiguredSnapshot = buildMeterData(displayMeter, engine, parameters);
    const auto expectedBandCount = getExpectedBandCount(sampleRate, parameters.engineParameters.bandMode);
    REQUIRE(reconfiguredSnapshot.bandInfo != nullptr);
    REQUIRE(reconfiguredSnapshot.bandInfo->size() == expectedBandCount);
    REQUIRE(reconfiguredSnapshot.traces.front().frame.peakDb.size() == expectedBandCount);
    requireStrongestBandNearFrequency(displayMeter, engine, parameters, highSineHz);
}

TEST_CASE("AnalyzerEngine keeps working after parameter changes") {
    Analyzer::Engine engine;
    AnalyzerMeter displayMeter;
    auto parameters = makeDefaultParameters();
    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::mid);
    parameters.engineParameters.bandMode = Analyzer::BandMode::octaveThird;

    prepareEngine(engine, parameters);
    processSineBlocks(engine, 1, {lowSineHz}, {1.0f});

    const auto initialSnapshot = buildMeterData(displayMeter, engine, parameters);
    REQUIRE(initialSnapshot.bandInfo != nullptr);
    REQUIRE(initialSnapshot.bandInfo->size() == getExpectedBandCount(sampleRate, parameters.engineParameters.bandMode));
    requireStrongestBandNearFrequency(displayMeter, engine, parameters, lowSineHz);

    setPrimarySignal(parameters, Analyzer::SignalSource::main, Analyzer::SignalMode::stereo);
    parameters.engineParameters.bandMode = Analyzer::BandMode::octaveSixth;
    engine.setParameters(parameters.engineParameters);

    processSineBlocks(engine, 2, {highSineHz, highSineHz}, {1.0f, -1.0f});

    const auto updatedSnapshot = buildMeterData(displayMeter, engine, parameters);
    const auto expectedBandCount = getExpectedBandCount(sampleRate, parameters.engineParameters.bandMode);
    REQUIRE(updatedSnapshot.bandInfo != nullptr);
    REQUIRE(updatedSnapshot.bandInfo->size() == expectedBandCount);
    REQUIRE(updatedSnapshot.traces.front().frame.peakDb.size() == expectedBandCount);
    requireStrongestBandNearFrequency(displayMeter, engine, parameters, highSineHz);
}
