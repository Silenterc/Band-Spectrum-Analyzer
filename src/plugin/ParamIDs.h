//
// Created by Lukáš Zima on 07.03.2026.
//

#ifndef BAND_SPECTRUM_ANALYZER_PARAMIDS_H
#define BAND_SPECTRUM_ANALYZER_PARAMIDS_H

namespace ParamIDs {
    inline constexpr const char* analysisMode = "analysisMode"; // Summed / MidSide
    inline constexpr const char* bandMode     = "bandMode";     // 30 / 40 / 60

    inline constexpr const char* showRms  = "showRms";
    inline constexpr const char* showPeak = "showPeak";
    inline constexpr const char* showHold = "showHold";

    inline constexpr const char* holdMs     = "holdMs";
    inline constexpr const char* gridMinDb  = "gridMinDb";
    inline constexpr const char* gridMaxDb  = "gridMaxDb";
    inline constexpr const char* gridStepDb = "gridStepDb";
}

#endif //BAND_SPECTRUM_ANALYZER_PARAMIDS_H