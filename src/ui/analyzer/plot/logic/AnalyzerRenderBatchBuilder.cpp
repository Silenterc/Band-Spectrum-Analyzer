#include "AnalyzerRenderBatchBuilder.h"

#include <optional>

#include "ui/analyzer/plot/logic/AnalyzerUiSelectors.h"

namespace {
    juce::Path buildSmoothedPathFromPoints(const std::vector<juce::Point<float>> &points) {
        juce::Path path;
        if (points.empty())
            return path;

        path.startNewSubPath(points.front());
        if (points.size() == 1)
            return path;

        if (points.size() == 2) {
            path.lineTo(points.back());
            return path;
        }

        for (size_t pointIndex = 1; pointIndex + 1 < points.size(); ++pointIndex) {
            const auto &current = points[pointIndex];
            const auto &next = points[pointIndex + 1];
            const auto midpoint = juce::Point<float>((current.x + next.x) * 0.5f,
                                                     (current.y + next.y) * 0.5f);
            path.quadraticTo(current, midpoint);
        }

        path.lineTo(points.back());
        return path;
    }

    std::optional<juce::Point<float>> getRmsPoint(const AnalyzerSlotDisplayFrame &slotFrame,
                                                  const AnalyzerVisibleBandLayout &visibleBand,
                                                  const float gridMinDb,
                                                  const float gridMaxDb,
                                                  const juce::Rectangle<float> &plotBounds,
                                                  const juce::Rectangle<float> &clipBounds) {
        if (!visibleBand.drawBounds.intersects(clipBounds))
            return std::nullopt;

        if (visibleBand.sourceBandIndex >= slotFrame.frame.rmsDb.size())
            return std::nullopt;

        const auto rmsDb = slotFrame.frame.rmsDb[visibleBand.sourceBandIndex];
        const auto clampedDb = juce::jlimit(gridMinDb, gridMaxDb, rmsDb);
        const auto normalised = juce::jmap(clampedDb, gridMinDb, gridMaxDb, 0.0f, 1.0f);
        const auto y = plotBounds.getBottom() - normalised * plotBounds.getHeight();
        return juce::Point<float>(visibleBand.drawBounds.getCentreX(), y);
    }

    juce::Path buildRmsPath(const AnalyzerSlotDisplayFrame &slotFrame,
                            const std::vector<AnalyzerVisibleBandLayout> &visibleBands,
                            const float gridMinDb,
                            const float gridMaxDb,
                            const juce::Rectangle<float> &plotBounds,
                            const juce::Rectangle<float> &clipBounds,
                            const size_t startIndex,
                            const size_t endIndexExclusive) {
        juce::Path path;
        std::vector<juce::Point<float>> segmentPoints;
        bool hasAudiblePoint = false;

        for (size_t visibleBandIndex = startIndex; visibleBandIndex < endIndexExclusive; ++visibleBandIndex) {
            const auto sourceBandIndex = visibleBands[visibleBandIndex].sourceBandIndex;
            if (sourceBandIndex < slotFrame.frame.rmsDb.size()
                && slotFrame.frame.rmsDb[sourceBandIndex] > gridMinDb) {
                hasAudiblePoint = true;
            }

            const auto point = getRmsPoint(slotFrame,
                                           visibleBands[visibleBandIndex],
                                           gridMinDb,
                                           gridMaxDb,
                                           plotBounds,
                                           clipBounds);
            if (!point.has_value()) {
                path.addPath(buildSmoothedPathFromPoints(segmentPoints));
                segmentPoints.clear();
                continue;
            }

            segmentPoints.push_back(*point);
        }

        if (!hasAudiblePoint)
            return path;

        path.addPath(buildSmoothedPathFromPoints(segmentPoints));
        return path;
    }
}

void AnalyzerRenderBatchBuilder::reset() {
    for (auto &batch: batches)
        batch.rectangles.clear();

    batches.clear();
    pathBatches.clear();
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

        for (const auto &visibleBand: visibleBands) {
            if (!visibleBand.drawBounds.intersects(clipBounds))
                continue;

            const auto peakDb = getBandValue(slotFrame.frame.peakDb, visibleBand.sourceBandIndex, gridMinDb);

            if (meterSettings.showPeak && peakDb > gridMinDb) {
                const auto peakY = yForDb(peakDb, gridMinDb, gridMaxDb, plotBounds);
                const auto peakBounds = juce::Rectangle<float>(visibleBand.drawBounds.getX(),
                                                               peakY,
                                                               visibleBand.drawBounds.getWidth(),
                                                               plotBounds.getBottom() - peakY).getSmallestIntegerContainer();
                if (!peakBounds.isEmpty())
                    addRect(peakColour, peakBounds);
            }
        }

        if (meterSettings.showRms) {
            const auto rmsColour = peakColour.withAlpha(1.0f)
                .interpolatedWith(juce::Colours::white, theme.metrics.analyzerPlot.rmsLineWhiteness)
                .withAlpha(theme.metrics.analyzerPlot.rmsLineAlpha);
            const auto rmsPath = buildRmsPath(slotFrame,
                                              visibleBands,
                                              gridMinDb,
                                              gridMaxDb,
                                              plotBounds,
                                              clipBounds,
                                              0,
                                              visibleBands.size());
            addPath(rmsColour, rmsPath, theme.metrics.analyzerPlot.rmsLineThickness);
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

        const auto ownerKind = visibleBand.sourceBandIndex < globalHoldFrame.ownerKinds.size()
                                   ? globalHoldFrame.ownerKinds[visibleBand.sourceBandIndex]
                                   : std::nullopt;
        if (!ownerKind.has_value())
            continue;

        const auto holdY = yForDb(holdDb, gridMinDb, gridMaxDb, plotBounds);
        const auto lineBounds = juce::Rectangle<float>(visibleBand.drawBounds.getX(),
                                                       holdY - 1.0f,
                                                       visibleBand.drawBounds.getWidth(),
                                                       2.0f).getSmallestIntegerContainer();
        if (!lineBounds.toFloat().intersects(clipBounds) || lineBounds.isEmpty())
            continue;

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
        const auto peakDb = getBandValue(slotFrame.frame.peakDb, visibleBand.sourceBandIndex, gridMinDb);

        if (meterSettings.showPeak && peakDb > gridMinDb) {
            const auto peakY = yForDb(peakDb, gridMinDb, gridMaxDb, plotBounds);
            const auto peakBounds = juce::Rectangle<float>(visibleBand.drawBounds.getX(),
                                                           peakY,
                                                           visibleBand.drawBounds.getWidth(),
                                                           plotBounds.getBottom() - peakY).getSmallestIntegerContainer();
            if (!peakBounds.isEmpty())
                addRect(peakColour, peakBounds);
        }

    }
}

const std::vector<AnalyzerRenderBatchBuilder::Batch> &AnalyzerRenderBatchBuilder::getBatches() const {
    return batches;
}

const std::vector<AnalyzerRenderBatchBuilder::PathBatch> &AnalyzerRenderBatchBuilder::getPathBatches() const {
    return pathBatches;
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

void AnalyzerRenderBatchBuilder::addPath(const juce::Colour &colour,
                                         const juce::Path &path,
                                         const float strokeThickness) {
    if (path.isEmpty())
        return;

    for (auto &batch: pathBatches) {
        if (batch.colour == colour && juce::approximatelyEqual(batch.strokeThickness, strokeThickness)) {
            batch.path.addPath(path);
            return;
        }
    }

    PathBatch batch;
    batch.colour = colour;
    batch.path = path;
    batch.strokeThickness = strokeThickness;
    pathBatches.push_back(std::move(batch));
}
