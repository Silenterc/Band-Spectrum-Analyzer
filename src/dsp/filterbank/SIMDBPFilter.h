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
     * SIMD cascaded band-pass filter that advances one packed band group in parallel.
     * The caller broadcasts one scalar input sample into all lanes. Internally, two
     * identical transposed-direct-form-II biquad stages are run in series so each lane
     * behaves like a 4th-order band-pass with steeper skirts than a single stage.
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
         * Clears both stage states for all SIMD lanes.
         */
        void reset();

        /**
         * Processes one broadcast input sample through all packed bands.
         */
        inline SimdFloat process(SimdFloat input) noexcept {
            const auto stage1Output = processStage(input, stage1s1, stage1s2);
            return processStage(stage1Output, stage2s1, stage2s2);
        }

    private:
        inline SimdFloat processStage(const SimdFloat input, SimdFloat &s1, SimdFloat &s2) noexcept {
            const auto y = params.b0 * input + s1;
            s1 = s2 + params.b1 * input - params.a1 * y;
            s2 = params.b2 * input - params.a2 * y;
            return y;
        }

        // Coefficients shared by the lifetime of this SIMD group.
        BandPassFilterParams params;
        // Per-lane state for the first cascaded biquad stage.
        SimdFloat stage1s1, stage1s2;
        // Per-lane state for the second cascaded biquad stage.
        SimdFloat stage2s1, stage2s2;
    };
}
