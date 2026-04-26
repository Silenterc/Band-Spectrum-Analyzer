#pragma once

#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "dsp/core/AnalyzerData.h"
#include "ui/analyzer/plot/state/AnalyzerSectionLayout.h"
#include "ui/theme/UiTheme.h"
#include "ui/analyzer/plot/state/AnalyzerViewState.h"
#include "ui/analyzer/plot/logic/AnalyzerGeometry.h"
#include "ui/analyzer/plot/logic/AnalyzerHoverModel.h"
#include "ui/analyzer/plot/logic/FrequencyFormatter.h"
#include "ui/analyzer/plot/logic/MusicTheory.h"

/**
 * Builds static analyzer UI state and hover readouts on the message thread
 */
class AnalyzerViewModel final {
public:
    explicit AnalyzerViewModel(const Ui::Theme &themeToUse);

    /**
     * Rebuilds the canonical analyzer layout shared by the shell and the plot child.
     */
    void updateLayout(const std::vector<Analyzer::BandInfo> &bandInfo,
                      const AnalyzerViewState &viewState,
                      float gridMinDb,
                      float gridMaxDb,
                      float gridStepDb,
                      const juce::Rectangle<float> &sectionBounds,
                      const juce::Rectangle<float> &displayBounds);

    /**
     * Builds hover state from the canonical layout and a plot-local cursor position.
     */
    [[nodiscard]] std::optional<AnalyzerHoverInfo> buildHover(
        const std::optional<juce::Point<float>> &plotLocalHoverPosition) const;

    /**
     * Returns the current canonical layout snapshot.
     */
    [[nodiscard]] const AnalyzerSectionLayout &getLayout() const;

private:
    void updateGrid(float gridMinDb, float gridMaxDb, float gridStepDb);
    void updateVisibleBands(const std::vector<Analyzer::BandInfo> &bandInfo);
    void updateVisibleFrequencyRange(const std::vector<Analyzer::BandInfo> &bandInfo,
                                     const AnalyzerViewState &viewState);

    const Ui::Theme &theme;
    AnalyzerGeometry geometry;
    FrequencyFormatter formatter;
    MusicTheory musicTheory;
    AnalyzerHoverModel hoverModel;
    AnalyzerSectionLayout layout;
};
