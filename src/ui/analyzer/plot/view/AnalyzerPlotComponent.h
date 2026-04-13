#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "display/analyzer/contracts/AnalyzerRawTraceSource.h"
#include "display/analyzer/data/AnalyzerDisplayControlState.h"
#include "display/analyzer/data/AnalyzerDisplayFrame.h"
#include "display/analyzer/thread/AnalyzerDisplayWorker.h"
#include "ui/analyzer/plot/data/AnalyzerSectionLayout.h"
#include "ui/analyzer/plot/logic/AnalyzerRenderBatchBuilder.h"
#include "ui/analyzer/state/AnalyzerUiSnapshot.h"
#include "ui/theme/UiTheme.h"

class AnalyzerPlotComponent final : public juce::Component,
                                    private juce::AsyncUpdater,
                                    private AnalyzerDisplayWorker::Listener {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void analyzerPlotHoverChanged(const std::optional<juce::Point<float>> &plotLocalPosition) = 0;
        virtual void analyzerPlotBandInfoChanged(
            const std::shared_ptr<const std::vector<Analyzer::BandInfo>> &bandInfo) = 0;
    };

    AnalyzerPlotComponent(AnalyzerRawTraceSource &rawTraceSource, const Ui::Theme &theme);
    ~AnalyzerPlotComponent() override;

    void setListener(Listener *listenerToUse);
    void setLayout(const AnalyzerSectionLayout &newLayout);
    void setUiSnapshot(const Ui::AnalyzerUiSnapshot &snapshot);
    void setHoveredBandIndex(const std::optional<size_t> &newHoveredBandIndex);

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &) override;

private:
    void drawGrid(juce::Graphics &g) const;
    void drawBars(juce::Graphics &g) const;
    void drawGlobalHold(juce::Graphics &g) const;
    void drawHoverHighlight(juce::Graphics &g) const;
    void ensureStaticPlotLayer();
    void rebuildDynamicBatches();
    void rebuildHoverHighlightBatches();
    void notifyHoverChanged(const std::optional<juce::Point<float>> &plotLocalPosition) const;
    [[nodiscard]] AnalyzerDisplayControlState makeDisplayControlState(const Ui::AnalyzerUiSnapshot &snapshot) const;
    [[nodiscard]] juce::Rectangle<int> getHoverRepaintBounds(const std::optional<size_t> &bandIndex) const;
    void handleAsyncUpdate() override;
    void analyzerDisplayFramePublished() override;

    const Ui::Theme &theme;
    AnalyzerDisplayWorker displayWorker;
    Listener *listener = nullptr;
    const AnalyzerDisplayFrame *displayFrame = nullptr;
    std::uint64_t lastConsumedRevision = 0;
    std::shared_ptr<const std::vector<Analyzer::BandInfo>> currentBandInfo;
    AnalyzerRenderBatchBuilder traceBatchBuilder;
    AnalyzerRenderBatchBuilder globalHoldBatchBuilder;
    AnalyzerRenderBatchBuilder hoverHighlightBatchBuilder;
    Ui::AnalyzerUiSnapshot uiSnapshot;
    bool hasUiSnapshot = false;
    std::optional<AnalyzerSectionLayout> layout;
    std::optional<size_t> hoveredBandIndex;
    juce::Image staticPlotLayer;
    bool staticPlotLayerDirty = true;
};
