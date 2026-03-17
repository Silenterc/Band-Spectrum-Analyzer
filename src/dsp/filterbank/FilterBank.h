//
// Created by Lukáš Zima on 16.03.2026.
//

#pragma once
#include <memory>
#include <vector>

#include "SIMDBPFilter.h"
#include "../core/AnalyzerData.h"
#include "../sources/AnalysisSourceBuilder.h"

namespace Analyzer {
    /**
     * Owns the packed SIMD biquads for one analysis trace.
     * Adjacent logical bands are grouped according to the current target's SIMD width so the
     * same broadcast input sample can advance multiple bands in parallel.
     */
    class FilterBank {
    public:
        enum class Mode {
            // One signal view feeds one packed bank.
            singleLane,
            // Two signal views are filtered independently and combined as averaged stereo power.
            stereoAverage
        };

        FilterBank() = default;

        /**
         * Rebuilds the packed band-pass filters for the current sample rate, band layout, and mode.
         */
        void prepare(double sampleRate, std::shared_ptr<const std::vector<BandInfo>> bandInfo, Mode modeToUse);

        /**
         * Clears filter state and measurement accumulators without changing the prepared layout.
         */
        void reset();

        /**
         * Runs one block through the prepared filters and overwrites the supplied per-band measurements.
         * `outputMeasurements` must already be sized for the current band layout.
         */
        void processBlock(const SignalView& primarySignalView,
                          const SignalView* secondarySignalView,
                          std::vector<BandMeasurements>& outputMeasurements);

    private:
        using SimdFloat = juce::dsp::SIMDRegister<float>;
        // TODO: Consider runtime dispatch so x86 builds can use wider AVX lanes when available
        // while keeping a conservative baseline path for distribution.
        static constexpr size_t groupWidth = SimdFloat::SIMDNumElements;

        /**
         * Calculates cookbook band-pass coefficients and packs them into SIMD groups.
         */
        void prepareFilters();

        /**
         * Processes one source view and accumulates per-lane power statistics.
         */
        void processBlockSingleLane(const SignalView& signalView,
                                    std::vector<BandMeasurements>& outputMeasurements);

        /**
         * Processes two source views and accumulates averaged stereo power per lane.
         */
        void processBlockStereoAverage(const SignalView& primarySignalView,
                                       const SignalView& secondarySignalView,
                                       std::vector<BandMeasurements>& outputMeasurements);

        /**
         * Copies SIMD lane accumulators back into the scalar band measurement storage.
         */
        void writeMeasurements(size_t bandAmount,
                               size_t numSamples,
                               std::vector<BandMeasurements>& outputMeasurements);

        /**
         * Clears block-local power accumulators while keeping filter delay state intact.
         */
        void clearPowerAccumulators();

        double sampleRate = 44100.0;
        Mode mode = Mode::singleLane;
        std::shared_ptr<const std::vector<BandInfo>> bandInfo;
        // Primary analysis path, always present.
        std::vector<SIMDBPFilter> primaryFilters;
        // Secondary analysis path used only in stereo-average mode.
        std::vector<SIMDBPFilter> secondaryFilters;
        // Per-group sum of squared output samples for the current block.
        std::vector<SimdFloat> sumPowers;
        // Per-group peak squared output sample for the current block.
        std::vector<SimdFloat> peakPowers;
    };
}
