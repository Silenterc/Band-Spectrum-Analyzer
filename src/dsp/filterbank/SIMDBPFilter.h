//
// Created by Lukáš Zima on 16.03.2026.
//

#pragma once
#include <array>
#include "juce_dsp/juce_dsp.h"

namespace Analyzer {
    /**
     * Packed coefficients for one SIMD band-pass group.
     * Each lane corresponds to one logical analyzer band.
     */
    struct BandPassFilterParams {
        juce::dsp::SIMDRegister<float> a1, a2;
        juce::dsp::SIMDRegister<float> b0, b1, b2;
    };

    /**
     * SIMD biquad that advances one packed band-pass group in parallel.
     * The caller broadcasts one scalar input sample into all lanes, and each lane keeps
     * its own transposed-direct-form-II state between calls.
     */
    class SIMDBPFilter {
    public:
        using SimdFloat = juce::dsp::SIMDRegister<float>;

        SIMDBPFilter() = default;

        /**
         * Replaces the packed coefficients and clears the internal delay state.
         */
        void prepare(const BandPassFilterParams& params);

        /**
         * Clears both state registers for all SIMD lanes.
         */
        void reset();

        /**
         * Processes one broadcast input sample through all packed bands.
         */
        inline SimdFloat process(SimdFloat input) noexcept {
            const auto y = params.b0 * input + s1;
            s1 = s2 + params.b1 * input - params.a1 * y;
            s2 = params.b2 * input - params.a2 * y;
            return y;
        }

    private:
        // Coefficients shared by the lifetime of this SIMD group.
        BandPassFilterParams params;
        // Per-lane filter state persisted across blocks.
        SimdFloat s1, s2;
    };
}
