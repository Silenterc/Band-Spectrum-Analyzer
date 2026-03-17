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
        // Each SIMD lane represents an independent biquad, so both state registers reset together.
        s1 = SimdFloat::expand(0.0f);
        s2 = SimdFloat::expand(0.0f);
    }
}
