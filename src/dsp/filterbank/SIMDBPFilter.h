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
        void reset();

        /**
         * Processes one SIMD sample frame through the transposed direct form II state update.
         */
        inline SimdFloat process(SimdFloat input) noexcept {
            const auto y = params.b0 * input + s1;
            s1 = s2 + params.b1 * input - params.a1 * y;
            s2 = params.b2 * input - params.a2 * y;
            return y;
        }

    private:
        BandPassFilterParams params;
        SimdFloat s1, s2;
    };
}
