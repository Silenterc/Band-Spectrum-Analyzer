#pragma once

#include <algorithm>
#include <optional>

#include "../../SignalSlotUiState.h"
#include "SignalSlotOptions.h"

namespace Ui {
    inline Shared::SignalSlotOrder appendSlotToEnd(const Shared::SignalSlotOrder &slotOrder, const size_t slotIndex) {
        Shared::SignalSlotOrder reordered = slotOrder;

        auto writeIt = std::remove(reordered.begin(), reordered.end(), slotIndex);
        std::fill(writeIt, reordered.end(), slotIndex);
        reordered[reordered.size() - 1] = slotIndex;

        size_t fillIndex = 0;
        for (auto readIt = reordered.begin(); readIt != reordered.end(); ++readIt) {
            if (*readIt == slotIndex && readIt != reordered.end() - 1)
                continue;

            reordered[fillIndex++] = *readIt;
        }

        return reordered;
    }

    inline std::optional<size_t> findFreeSignalSlot(
        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots) {
        for (size_t slotIndex = 0; slotIndex < signalSlots.size(); ++slotIndex) {
            if (!signalSlots[slotIndex].configuration.enabled)
                return slotIndex;
        }

        return std::nullopt;
    }

    inline bool isSignalSlotKeyUsed(const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                    const Analyzer::SignalSource source,
                                    const Analyzer::SignalMode mode) {
        return std::any_of(signalSlots.begin(), signalSlots.end(),
                           [source, mode](const Ui::SignalSlotState &slot) {
                               return slot.configuration.enabled
                                      && slot.configuration.source == source
                                      && slot.configuration.mode == mode;
                           });
    }

    inline Analyzer::SignalSlotConfiguration chooseDefaultSignalConfiguration(
        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
        const bool sidechainAvailable) {
        for (const auto &option: signalSlotOptions) {
            if (option.requiresSidechain && !sidechainAvailable)
                continue;

            if (!isSignalSlotKeyUsed(signalSlots, option.source, option.mode)) {
                Analyzer::SignalSlotConfiguration configuration;
                configuration.enabled = true;
                configuration.source = option.source;
                configuration.mode = option.mode;
                return configuration;
            }
        }

        Analyzer::SignalSlotConfiguration configuration;
        configuration.enabled = true;
        configuration.source = Analyzer::SignalSource::main;
        configuration.mode = Analyzer::SignalMode::mid;
        return configuration;
    }

    inline int chooseDefaultSignalColourIndex(
        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots) {
        for (int colourIndex = 0; colourIndex < Ui::signalPresetCount; ++colourIndex) {
            const auto colourInUse = std::any_of(signalSlots.begin(), signalSlots.end(),
                                                 [colourIndex](const Ui::SignalSlotState &slot) {
                                                     return slot.configuration.enabled && slot.colourIndex == colourIndex;
                                                 });
            if (!colourInUse)
                return colourIndex;
        }

        return 0;
    }
}
