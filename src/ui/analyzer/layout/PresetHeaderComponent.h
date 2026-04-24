#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/analyzer/layout/PresetHeaderControlsComponent.h"
#include "ui/contracts/PresetActions.h"
#include "ui/contracts/PresetUiSnapshotSource.h"
#include "ui/theme/UiTheme.h"

class PresetHeaderComponent final : public juce::Component {
public:
    PresetHeaderComponent(PresetUiSnapshotSource& presetUiSnapshotSourceToUse,
                          PresetActions& presetActionsToUse,
                          const Ui::Theme& themeToUse);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    [[nodiscard]] juce::Rectangle<int> getLogoBounds(juce::Rectangle<int> availableBounds) const;

    const Ui::Theme& theme;
    PresetHeaderControlsComponent controlsComponent;
    juce::Rectangle<int> logoBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetHeaderComponent)
};
