//
// Created by Lukáš Zima on 16.03.2026.
//

#pragma once
#include <array>
#include "juce_dsp/juce_dsp.h"

namespace Analyzer {
    /**
     * Parameters for a constant 0 dB peak gain BPF.
     * From The EQ Cookbook
     */
    struct BandPassFilterParams {
        juce::dsp::SIMDRegister<float> a1, a2;
        juce::dsp::SIMDRegister<float> b0, b1, b2;
    };

    /**
     * SIMD biquad that processes 4 independent bands in parallel using transposed direct form II.
     */
    class SIMDBPFilter {
    public:
        using SimdFloat = juce::dsp::SIMDRegister<float>;

        SIMDBPFilter() = default;

        void prepare(const BandPassFilterParams& params);

        /**
         * Processes one SIMD sample frame through the transposed direct form II state update.
         */
        SimdFloat process(const SimdFloat& input);

        SimdFloat process(float inputSample) {
            return process(SimdFloat(inputSample));
        }

    private:
        BandPassFilterParams params;
        SimdFloat s1, s2;
    };
}
