#pragma once

#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/presets/contracts/PresetActions.h"
#include "ui/presets/contracts/PresetUiSnapshotSource.h"
#include "ui/presets/popups/PresetSavePopupContent.h"
#include "ui/presets/view/PresetBrowserPopupComponent.h"
#include "ui/presets/view/PresetSelectorButton.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/CalloutPresenter.h"
#include "ui/widgets/PopupLookAndFeel.h"
#include "ui/widgets/RasterIconButton.h"

class PresetHeaderControlsComponent final : public juce::Component,
                                            private PresetUiSnapshotSource::Listener {
public:
    PresetHeaderControlsComponent(PresetUiSnapshotSource& presetUiSnapshotSourceToUse,
                                  PresetActions& presetActionsToUse,
                                  const Ui::Theme& themeToUse);
    ~PresetHeaderControlsComponent() override;

    [[nodiscard]] int getPreferredWidth() const;

    void resized() override;

private:
    enum class OpenPopup {
        none,
        browser,
        save
    };

    void presetUiSnapshotChanged(const Ui::Presets::PresetUiSnapshot& snapshot) override;
    void updateButtonState();
    void dismissOpenPopup();
    void launchCallout(std::unique_ptr<juce::Component> content,
                       OpenPopup popupKind,
                       juce::Component& triggerComponent);
    void showBrowserPopup();
    void showSavePopup();
    void handleBrowserTriggerPressed();
    void handleSaveTriggerPressed();
    void handlePopupDismissed(OpenPopup popupKind);
    void focusPopupContent(juce::Component* content) const;
    [[nodiscard]] juce::Component* getCalloutParentComponent() const;
    [[nodiscard]] juce::Rectangle<int> makeCalloutAnchorBounds(juce::Component& triggerComponent) const;
    [[nodiscard]] int getTextBoxPreferredWidth() const;
    [[nodiscard]] juce::Rectangle<int> getScaledTextBoxBounds(juce::Rectangle<int> availableBounds) const;

    PresetUiSnapshotSource& presetUiSnapshotSource;
    PresetActions& presetActions;
    const Ui::Theme& theme;
    PopupLookAndFeel popupLookAndFeel;
    Ui::CalloutPresenter calloutPresenter;
    RasterIconButton previousButton;
    PresetSelectorButton selectorButton;
    RasterIconButton nextButton;
    RasterIconButton resetButton;
    RasterIconButton saveButton;
    Ui::Presets::PresetUiSnapshot presetUiSnapshot;
    OpenPopup openPopup = OpenPopup::none;
    bool suppressNextSaveButtonClick = false;
    bool suppressNextSelectorClick = false;
    juce::Component::SafePointer<juce::Component> popupTriggerComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetHeaderControlsComponent)
};
