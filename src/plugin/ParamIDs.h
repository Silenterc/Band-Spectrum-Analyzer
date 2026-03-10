#pragma once

namespace ParamIDs {
    inline constexpr const char* analysisMode = "analysisMode"; // Mid / Side / Stereo, main or sidechain
    inline constexpr const char* bandMode     = "bandMode";     // 30 / 40 / 60

    inline constexpr const char* showRms  = "showRms";
    inline constexpr const char* showPeak = "showPeak";
    inline constexpr const char* showHold = "showHold";

    inline constexpr const char* holdMs     = "holdMs";
    inline constexpr const char* gridMinDb  = "gridMinDb";
    inline constexpr const char* gridMaxDb  = "gridMaxDb";
    inline constexpr const char* gridStepDb = "gridStepDb";
}
