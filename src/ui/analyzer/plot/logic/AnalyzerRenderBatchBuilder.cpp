#include "AnalyzerRenderBatchBuilder.h"

#include "ui/analyzer/plot/logic/AnalyzerUiSelectors.h"

void AnalyzerRenderBatchBuilder::reset() {
    for (auto &batch: batches)
        batch.rectangles.clear();

    batches.clear();
}

void AnalyzerRenderBatchBuilder::buildTraceBatches(const AnalyzerDisplayFrame *displayFrame,
                                                   const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                                                   const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                                   const Shared::SignalSlotOrder &signalSlotOrder,
                                                   const Analyzer::MeterSettings &meterSettings,
                                                   const float gridMinDb,
                                                   const float gridMaxDb,
                                                   const juce::Rectangle<float> &plotBounds,
                                                   const juce::Rectangle<float> &clipBounds) {
    reset();

    if (displayFrame == nullptr)
        return;

    // Visual order and styling are applied here after the worker publishes semantic slot frames.
    for (const auto slotIndex: signalSlotOrder) {
        if (slotIndex >= signalSlots.size() || slotIndex >= displayFrame->slotFrames.size())
            continue;

        const auto &slot = signalSlots[slotIndex];
        const auto &slotFrame = displayFrame->slotFrames[slotIndex];
        if (!slot.configuration.enabled || !slot.visible || !slotFrame.active)
            continue;

        const auto peakColour = getTraceColour(slot);
        const auto rmsColour = peakColour.withMultipliedAlpha(0.45f);

        for (const auto &visibleBand: visibleBands) {
            if (!visibleBand.drawBounds.intersects(clipBounds))
                continue;

            const auto peakDb = getBandValue(slotFrame.frame.peakDb, visibleBand.sourceBandIndex, gridMinDb);
            const auto rmsDb = getBandValue(slotFrame.frame.rmsDb, visibleBand.sourceBandIndex, gridMinDb);

            if (meterSettings.showPeak && peakDb > gridMinDb) {
                const auto peakY = yForDb(peakDb, gridMinDb, gridMaxDb, plotBounds);
                const auto peakBounds = juce::Rectangle<float>(visibleBand.drawBounds.getX(),
                                                               peakY,
                                                               visibleBand.drawBounds.getWidth(),
                                                               plotBounds.getBottom() - peakY).getSmallestIntegerContainer();
                if (!peakBounds.isEmpty())
                    addRect(peakColour, peakBounds);
            }

            if (meterSettings.showRms && rmsDb > gridMinDb) {
                const auto rmsY = yForDb(rmsDb, gridMinDb, gridMaxDb, plotBounds);
                const auto rmsBounds = juce::Rectangle<float>(visibleBand.drawBounds.getX(),
                                                              rmsY,
                                                              visibleBand.drawBounds.getWidth(),
                                                              plotBounds.getBottom() - rmsY).getSmallestIntegerContainer();
                if (!rmsBounds.isEmpty())
                    addRect(rmsColour, rmsBounds);
            }
        }
    }
}

void AnalyzerRenderBatchBuilder::buildGlobalHoldBatches(const AnalyzerDisplayFrame *displayFrame,
                                                        const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                                                        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                                        const Analyzer::MeterSettings &meterSettings,
                                                        const float gridMinDb,
                                                        const float gridMaxDb,
                                                        const juce::Rectangle<float> &plotBounds,
                                                        const juce::Rectangle<float> &clipBounds) {
    reset();

    if (displayFrame == nullptr || !meterSettings.showHold || !displayFrame->globalHoldFrame.has_value())
        return;

    const auto &globalHoldFrame = *displayFrame->globalHoldFrame;
    for (const auto &visibleBand: visibleBands) {
        if (visibleBand.sourceBandIndex >= globalHoldFrame.holdDb.size())
            continue;

        const auto holdDb = globalHoldFrame.holdDb[visibleBand.sourceBandIndex];
        if (holdDb <= gridMinDb)
            continue;

        const auto holdY = yForDb(holdDb, gridMinDb, gridMaxDb, plotBounds);
        const auto lineBounds = juce::Rectangle<float>(visibleBand.drawBounds.getX(),
                                                       holdY - 1.0f,
                                                       visibleBand.drawBounds.getWidth(),
                                                       2.0f).getSmallestIntegerContainer();
        if (!lineBounds.toFloat().intersects(clipBounds) || lineBounds.isEmpty())
            continue;

        const auto ownerKind = visibleBand.sourceBandIndex < globalHoldFrame.ownerKinds.size()
                                   ? globalHoldFrame.ownerKinds[visibleBand.sourceBandIndex]
                                   : std::nullopt;
        addRect(makeGlobalHoldColour(ownerKind, signalSlots), lineBounds);
    }
}

void AnalyzerRenderBatchBuilder::buildHoveredBarBatches(const AnalyzerDisplayFrame *displayFrame,
                                                        const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                                                        const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots,
                                                        const Shared::SignalSlotOrder &signalSlotOrder,
                                                        const Analyzer::MeterSettings &meterSettings,
                                                        const size_t hoveredBandIndex,
                                                        const float gridMinDb,
                                                        const float gridMaxDb,
                                                        const juce::Rectangle<float> &plotBounds,
                                                        const juce::Rectangle<float> &clipBounds) {
    reset();

    if (displayFrame == nullptr || hoveredBandIndex >= visibleBands.size())
        return;

    const auto &visibleBand = visibleBands[hoveredBandIndex];
    if (!visibleBand.drawBounds.intersects(clipBounds))
        return;

    for (const auto slotIndex: signalSlotOrder) {
        if (slotIndex >= signalSlots.size() || slotIndex >= displayFrame->slotFrames.size())
            continue;

        const auto &slot = signalSlots[slotIndex];
        const auto &slotFrame = displayFrame->slotFrames[slotIndex];
        if (!slot.configuration.enabled || !slot.visible || !slotFrame.active)
            continue;

        const auto peakColour = getTraceColour(slot).brighter(0.18f);
        const auto rmsColour = peakColour.withMultipliedAlpha(0.45f);
        const auto peakDb = getBandValue(slotFrame.frame.peakDb, visibleBand.sourceBandIndex, gridMinDb);
        const auto rmsDb = getBandValue(slotFrame.frame.rmsDb, visibleBand.sourceBandIndex, gridMinDb);

        if (meterSettings.showPeak && peakDb > gridMinDb) {
            const auto peakY = yForDb(peakDb, gridMinDb, gridMaxDb, plotBounds);
            const auto peakBounds = juce::Rectangle<float>(visibleBand.drawBounds.getX(),
                                                           peakY,
                                                           visibleBand.drawBounds.getWidth(),
                                                           plotBounds.getBottom() - peakY).getSmallestIntegerContainer();
            if (!peakBounds.isEmpty())
                addRect(peakColour, peakBounds);
        }

        if (meterSettings.showRms && rmsDb > gridMinDb) {
            const auto rmsY = yForDb(rmsDb, gridMinDb, gridMaxDb, plotBounds);
            const auto rmsBounds = juce::Rectangle<float>(visibleBand.drawBounds.getX(),
                                                          rmsY,
                                                          visibleBand.drawBounds.getWidth(),
                                                          plotBounds.getBottom() - rmsY).getSmallestIntegerContainer();
            if (!rmsBounds.isEmpty())
                addRect(rmsColour, rmsBounds);
        }
    }
}

const std::vector<AnalyzerRenderBatchBuilder::Batch> &AnalyzerRenderBatchBuilder::getBatches() const {
    return batches;
}

float AnalyzerRenderBatchBuilder::yForDb(const float decibels,
                                         const float minDb,
                                         const float maxDb,
                                         const juce::Rectangle<float> &plotBounds) {
    const auto clampedDb = juce::jlimit(minDb, maxDb, decibels);
    const auto normalised = juce::jmap(clampedDb, minDb, maxDb, 0.0f, 1.0f);
    return plotBounds.getBottom() - normalised * plotBounds.getHeight();
}

float AnalyzerRenderBatchBuilder::getBandValue(const std::vector<float> &values,
                                               const size_t bandIndex,
                                               const float floorDb) {
    if (bandIndex >= values.size())
        return floorDb;

    return values[bandIndex];
}

juce::Colour AnalyzerRenderBatchBuilder::getTraceColour(const Ui::SignalSlotState &slot) {
    return Ui::getSignalPresetColour(slot.colourIndex).withAlpha(slot.opacity);
}

juce::Colour AnalyzerRenderBatchBuilder::makeGlobalHoldColour(
    const std::optional<Analyzer::TraceKind> &ownerKind,
    const std::array<Ui::SignalSlotState, Shared::maxSignalSlots> &signalSlots) const {
    juce::Colour baseColour = juce::Colours::white;

    if (ownerKind.has_value()) {
        if (const auto slotIndex = Analyzer::slotIndexForTraceKind(*ownerKind); slotIndex.has_value()) {
            const auto &slot = signalSlots[*slotIndex];
            baseColour = getTraceColour(slot);
        }
    }

    return Ui::makeHoldIndicatorColour(baseColour, theme);
}

void AnalyzerRenderBatchBuilder::addRect(const juce::Colour &colour, const juce::Rectangle<int> &bounds) {
    for (auto &batch: batches) {
        if (batch.colour == colour) {
            batch.rectangles.add(bounds);
            return;
        }
    }

    Batch batch;
    batch.colour = colour;
    batch.rectangles.add(bounds);
    batches.push_back(std::move(batch));
}
