#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "display/analyzer/contracts/AnalyzerRawTraceSource.h"
#include "ui/contracts/AnalyzerUiSnapshotSource.h"
#include "ui/theme/UiTheme.h"
#include "ui/analyzer/plot/view/AnalyzerComponent.h"

class AnalyzerSectionComponent final : public juce::Component {
public:
    AnalyzerSectionComponent(AnalyzerRawTraceSource& rawTraceSource,
                             AnalyzerUiSnapshotSource& snapshotSource,
                             const Ui::Theme& theme);
    ~AnalyzerSectionComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    struct Layout {
        juce::Rectangle<int> displayBounds;
    };

    Layout computeLayout() const;
    void rebuildCachedBackground();

    const Ui::Theme& theme;
    AnalyzerComponent analyzerDisplayComponent;
    juce::Image cachedBackground;
};
