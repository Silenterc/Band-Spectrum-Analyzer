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
     * Groups SIMD biquads into a band analyzer that processes 4 bands per filter instance.
     */
    class FilterBank {
    public:
        FilterBank() = default;

        void prepare(double sampleRate, std::shared_ptr<const std::vector<BandInfo>> bandInfo);

        void reset();

        /**
         * Runs the prepared filter bank over one signal block and accumulates per-band measurements.
         */
        void processBlock(const SignalView& signalView, std::vector<BandMeasurements>& outputMeasurements);

    private:
        using SimdFloat = juce::dsp::SIMDRegister<float>;

        /**
         * Calculates necessary BPF params according to the EQ Cookbook and creates the filters
         */
        void prepareFilters();

        double sampleRate;
        std::shared_ptr<const std::vector<BandInfo>> bandInfo;
        std::vector<SIMDBPFilter> bandPassFilters;
        std::vector<SimdFloat> sumPowers;
        std::vector<SimdFloat> peakPowers;
    };
}
