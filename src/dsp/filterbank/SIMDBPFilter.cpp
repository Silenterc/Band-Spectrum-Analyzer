//
// Created by Lukáš Zima on 16.03.2026.
//

#include "SIMDBPFilter.h"

namespace Analyzer {
    void SIMDBPFilter::prepare(const BandPassFilterParams& newParams) {
        params = newParams;
        s1 = SimdFloat::expand(0.0f);
        s2 = SimdFloat::expand(0.0f);
    }

    SIMDBPFilter::SimdFloat SIMDBPFilter::process(const SimdFloat& input) {
        const auto y = params.b0 * input + s1;
        s1 = s2 + params.b1 * input - params.a1 * y;
        s2 = params.b2 * input - params.a2 * y;
        return y;
    }
}
