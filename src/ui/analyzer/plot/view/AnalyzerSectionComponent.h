#pragma once

#include <memory>
#include <optional>

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "display/analyzer/contracts/AnalyzerRawTraceSource.h"
#include "ui/contracts/AnalyzerUiSnapshotSource.h"
#include "ui/theme/UiTheme.h"
#include "ui/analyzer/plot/data/AnalyzerViewState.h"
#include "ui/analyzer/plot/logic/AnalyzerViewModel.h"
#include "ui/analyzer/plot/view/AnalyzerHoverTooltipRenderer.h"
#include "ui/analyzer/plot/view/AnalyzerPlotComponent.h"

class AnalyzerSectionComponent final : public juce::Component,
                                       private AnalyzerUiSnapshotSource::Listener,
                                       private AnalyzerPlotComponent::Listener {
public:
    AnalyzerSectionComponent(AnalyzerRawTraceSource &rawTraceSource,
                             AnalyzerUiSnapshotSource &snapshotSource,
                             const Ui::Theme &theme);
    ~AnalyzerSectionComponent() override;

    void paint(juce::Graphics &g) override;
    void paintOverChildren(juce::Graphics &g) override;
    void resized() override;

private:
    struct Layout {
        juce::Rectangle<int> displayBounds;
    };

    void drawAxisLabels(juce::Graphics &g) const;
    void drawPlotChrome(juce::Graphics &g) const;
    void rebuildLayout();
    void rebuildCachedBackground();
    void updateHoverPresentation();
    [[nodiscard]] Layout computeLayout() const;
    void analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) override;
    void analyzerPlotHoverChanged(const std::optional<juce::Point<float>> &plotLocalPosition) override;
    void analyzerPlotBandInfoChanged(const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo) override;

    AnalyzerRawTraceSource &rawTraceSource;
    AnalyzerUiSnapshotSource &snapshotSource;
    const Ui::Theme &theme;
    AnalyzerPlotComponent analyzerPlotComponent;
    AnalyzerViewModel viewModel;
    AnalyzerHoverTooltipRenderer hoverTooltipRenderer;
    AnalyzerViewState viewState;
    Ui::AnalyzerUiSnapshot uiSnapshot;
    std::shared_ptr<const std::vector<Analyzer::BandInfo>> currentBandInfo;
    std::optional<juce::Point<float>> hoverPosition;
    juce::Image cachedBackground;
};
