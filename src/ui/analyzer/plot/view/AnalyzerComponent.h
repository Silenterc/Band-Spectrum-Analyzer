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
#include "ui/contracts/AnalyzerUiSnapshotSource.h"
#include "ui/theme/UiTheme.h"
#include "ui/analyzer/plot/data/AnalyzerViewState.h"
#include "ui/analyzer/plot/logic/AnalyzerRenderBatchBuilder.h"
#include "ui/analyzer/plot/logic/AnalyzerViewModel.h"
#include "ui/analyzer/plot/view/AnalyzerHoverOverlayRenderer.h"

class AnalyzerComponent final : public juce::Component,
                                private juce::AsyncUpdater,
                                private AnalyzerUiSnapshotSource::Listener,
                                private AnalyzerDisplayWorker::Listener {
public:
    AnalyzerComponent(AnalyzerRawTraceSource &rawTraceSource,
                      AnalyzerUiSnapshotSource &snapshotSource,
                      const Ui::Theme &theme);
    ~AnalyzerComponent() override;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &) override;

private:
    struct StaticViewStateKey {
        juce::Rectangle<int> bounds;
        size_t bandCount = 0;
        float minBandFrequencyHz = 0.0f;
        float maxBandFrequencyHz = 0.0f;
        float gridMinDb = 0.0f;
        float gridMaxDb = 0.0f;
        float gridStepDb = 0.0f;
        bool useCustomFrequencyRange = false;
        float visibleMinFrequencyHz = 0.0f;
        float visibleMaxFrequencyHz = 0.0f;

        bool operator==(const StaticViewStateKey &other) const;
    };

    void drawGrid(juce::Graphics &g) const;
    void drawBars(juce::Graphics &g) const;
    void drawGlobalHold(juce::Graphics &g) const;
    void drawHover(juce::Graphics &g) const;
    [[nodiscard]] juce::Rectangle<int> rebuildPresentationModel();
    void refreshStaticViewModelIfNeeded();
    void rebuildDynamicBatches();
    [[nodiscard]] juce::Rectangle<int> updateHoverState();
    void ensureStaticLayer();
    void repaintAnalyzer(const juce::Rectangle<int> &additionalBounds, bool includeDynamicBounds);
    [[nodiscard]] juce::Rectangle<int> getDynamicRepaintBounds() const;
    [[nodiscard]] std::shared_ptr<const std::vector<Analyzer::BandInfo>> getCurrentBandInfo() const;
    [[nodiscard]] AnalyzerDisplayControlState makeDisplayControlState(const Ui::AnalyzerUiSnapshot &snapshot) const;
    void handleAsyncUpdate() override;
    void analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) override;
    void analyzerDisplayFramePublished() override;
    void requestHoverUpdate();

    AnalyzerRawTraceSource &rawTraceSource;
    AnalyzerUiSnapshotSource &snapshotSource;
    const Ui::Theme &theme;
    AnalyzerDisplayWorker displayWorker;
    const AnalyzerDisplayFrame *displayFrame = nullptr;
    std::uint64_t lastConsumedRevision = 0;
    std::shared_ptr<const std::vector<Analyzer::BandInfo>> currentBandInfo;
    AnalyzerViewModel viewModel;
    AnalyzerHoverOverlayRenderer hoverOverlay;
    AnalyzerRenderBatchBuilder traceBatchBuilder;
    AnalyzerRenderBatchBuilder globalHoldBatchBuilder;
    AnalyzerViewState viewState;
    Ui::AnalyzerUiSnapshot uiSnapshot;
    std::optional<juce::Point<float>> hoverPosition;
    bool hoverUpdatePending = false;
    std::uint64_t hoverVisualRevision = 1;
    juce::Image staticLayer;
    bool staticLayerDirty = true;
    std::optional<StaticViewStateKey> staticViewStateKey;
};
