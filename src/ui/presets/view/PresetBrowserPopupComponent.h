#pragma once

#include <functional>
#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/presets/popups/PresetBrowserPopupContent.h"
#include "ui/presets/popups/PresetDeleteConfirmPopupContent.h"
#include "ui/presets/state/PresetUiState.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/CalloutPresenter.h"
#include "ui/widgets/PopupLookAndFeel.h"

class PresetBrowserPopupComponent final : public juce::Component {
public:
    struct Callbacks {
        std::function<Ui::Presets::PresetActionResult(const Ui::Presets::PresetId&)> onLoadPreset;
        std::function<Ui::Presets::PresetActionResult(const Ui::Presets::PresetId&)> onDeletePreset;
        std::function<void()> onCloseRequested;
        std::function<void()> onDismissed;
    };

    PresetBrowserPopupComponent(const Ui::Theme& themeToUse,
                                Ui::Presets::PresetUiSnapshot uiSnapshotToUse,
                                Callbacks callbacksToUse);
    ~PresetBrowserPopupComponent() override;

    [[nodiscard]] int getPreferredWidth() const;
    [[nodiscard]] int getPreferredHeight() const;
    void focusInitialControl();

    void resized() override;

private:
    void dismissDeleteConfirmation();
    void launchDeleteConfirmation(const Ui::Presets::PresetDescriptor& descriptor,
                                  const PresetListRowComponent& row,
                                  juce::Rectangle<int> deleteBounds);

    const Ui::Theme& theme;
    Callbacks callbacks;
    PopupLookAndFeel popupLookAndFeel;
    Ui::CalloutPresenter deleteCalloutPresenter;
    PresetBrowserPopupContent content;
    std::optional<Ui::Presets::PresetId> activeDeletePresetId;
};
