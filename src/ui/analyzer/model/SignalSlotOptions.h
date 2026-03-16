#pragma once

#include <array>
#include <utility>

#include "../../../shared/SignalSlotConfiguration.h"

namespace Ui {
    using SignalSlotKey = std::pair<Analyzer::SignalSource, Analyzer::SignalMode>;

    struct SignalSlotOption {
        const char *label = "";
        Analyzer::SignalSource source = Analyzer::SignalSource::main;
        Analyzer::SignalMode mode = Analyzer::SignalMode::mid;
        bool requiresSidechain = false;
    };

    inline constexpr std::array<SignalSlotOption, 6> signalSlotOptions{{
        {"Mid", Analyzer::SignalSource::main, Analyzer::SignalMode::mid, false},
        {"Side", Analyzer::SignalSource::main, Analyzer::SignalMode::side, false},
        {"Stereo", Analyzer::SignalSource::main, Analyzer::SignalMode::stereo, false},
        {"Mid", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::mid, true},
        {"Side", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::side, true},
        {"Stereo", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::stereo, true},
    }};

    inline constexpr const char *getSignalModeLabel(const Analyzer::SignalMode mode) {
        switch (mode) {
            case Analyzer::SignalMode::mid:
                return "Mid";
            case Analyzer::SignalMode::side:
                return "Side";
            case Analyzer::SignalMode::stereo:
                return "Stereo";
        }

        return "Mid";
    }

    inline constexpr const char *getSignalSourceHint(const Analyzer::SignalSource source) {
        return source == Analyzer::SignalSource::main ? "Main" : "Sidechain";
    }

    inline constexpr SignalSlotKey makeSignalSlotKey(const Analyzer::SignalSource source, const Analyzer::SignalMode mode) {
        return {source, mode};
    }
}
