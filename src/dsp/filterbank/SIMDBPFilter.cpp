//
// Created by Lukáš Zima on 16.03.2026.
//

#include "SIMDBPFilter.h"

namespace Analyzer {
    void SIMDBPFilter::prepare(const BandPassFilterParams& newParams) {
        params = newParams;
        reset();
    }

    void SIMDBPFilter::reset() {
        // Each SIMD lane represents an independent cascaded filter, so both stages reset together.
        stage1s1 = SimdFloat::expand(0.0f);
        stage1s2 = SimdFloat::expand(0.0f);
        stage2s1 = SimdFloat::expand(0.0f);
        stage2s2 = SimdFloat::expand(0.0f);
    }
}
