#pragma once

#include <array>
#include <utility>

#include "shared/SignalSlotConfiguration.h"

namespace Ui {
    using SignalSlotKey = std::pair<Analyzer::SignalSource, Analyzer::SignalMode>;

    struct SignalSlotOption {
        const char *modeLabel = "";
        const char *sourceLabel = "";
        const char *sourceHint = "";
        Analyzer::SignalSource source = Analyzer::SignalSource::main;
        Analyzer::SignalMode mode = Analyzer::SignalMode::mid;
        bool requiresSidechain = false;
    };

    inline constexpr std::array<SignalSlotOption, 6> signalSlotOptions{{
        {"Mid", "Main", "Main", Analyzer::SignalSource::main, Analyzer::SignalMode::mid, false},
        {"Side", "Main", "Main", Analyzer::SignalSource::main, Analyzer::SignalMode::side, false},
        {"Stereo", "Main", "Main", Analyzer::SignalSource::main, Analyzer::SignalMode::stereo, false},
        {"Mid", "Sidechain", "Sidechain", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::mid, true},
        {"Side", "Sidechain", "Sidechain", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::side, true},
        {"Stereo", "Sidechain", "Sidechain", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::stereo, true},
    }};

    inline constexpr const SignalSlotOption *findSignalSlotOption(const Analyzer::SignalSource source,
                                                                  const Analyzer::SignalMode mode) {
        for (const auto &option : signalSlotOptions) {
            if (option.source == source && option.mode == mode)
                return &option;
        }

        return nullptr;
    }

    inline constexpr const char *getSignalModeLabel(const Analyzer::SignalMode mode) {
        if (const auto *option = findSignalSlotOption(Analyzer::SignalSource::main, mode))
            return option->modeLabel;

        return signalSlotOptions.front().modeLabel;
    }

    inline constexpr const char *getSignalSourceLabel(const Analyzer::SignalSource source) {
        for (const auto &option : signalSlotOptions) {
            if (option.source == source)
                return option.sourceLabel;
        }

        return signalSlotOptions.front().sourceLabel;
    }

    inline constexpr const char *getSignalSourceHint(const Analyzer::SignalSource source) {
        for (const auto &option : signalSlotOptions) {
            if (option.source == source)
                return option.sourceHint;
        }

        return signalSlotOptions.front().sourceHint;
    }

    inline constexpr size_t getVisibleSignalSlotOptionCount(const Analyzer::SignalSource source,
                                                            const bool sidechainAvailable) {
        size_t count = 0;
        for (const auto &option : signalSlotOptions) {
            if (option.source != source)
                continue;
            if (option.requiresSidechain && !sidechainAvailable)
                continue;
            ++count;
        }

        return count;
    }

    inline constexpr SignalSlotKey makeSignalSlotKey(const Analyzer::SignalSource source, const Analyzer::SignalMode mode) {
        return {source, mode};
    }
}
