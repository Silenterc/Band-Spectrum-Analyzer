#include "PresetHeaderControlsComponent.h"

#include "ui/theme/UiRasterAssets.h"

PresetHeaderControlsComponent::PresetHeaderControlsComponent(PresetUiSnapshotSource& presetUiSnapshotSourceToUse,
                                                             PresetActions& presetActionsToUse,
                                                             const Ui::Theme& themeToUse)
    : presetUiSnapshotSource(presetUiSnapshotSourceToUse),
      presetActions(presetActionsToUse),
      theme(themeToUse),
      popupLookAndFeel(themeToUse),
      previousButton(themeToUse),
      selectorButton(themeToUse),
      nextButton(themeToUse),
      resetButton(themeToUse),
      saveButton(themeToUse) {
    previousButton.setIcon(Ui::IconId::left);
    nextButton.setIcon(Ui::IconId::right);
    resetButton.setIcon(Ui::IconId::reset);
    saveButton.setIcon(Ui::IconId::save);

    previousButton.setTooltip("Previous preset");
    nextButton.setTooltip("Next preset");
    resetButton.setTooltip("Reset preset");
    saveButton.setTooltip("Save preset");
    selectorButton.setTooltip("Preset browser");
    previousButton.onClick = [this] { static_cast<void>(presetActions.loadPreviousPreset()); };
    nextButton.onClick = [this] { static_cast<void>(presetActions.loadNextPreset()); };
    resetButton.onClick = [this] { static_cast<void>(presetActions.resetCurrentPreset()); };
    selectorButton.onPressed = [this] { handleBrowserTriggerPressed(); };
    selectorButton.onDownArrow = [this] {
        if (openPopup == OpenPopup::browser)
            return;

        showBrowserPopup();
    };
    selectorButton.onClick = [this] {
        if (suppressNextSelectorClick) {
            suppressNextSelectorClick = false;
            return;
        }

        showBrowserPopup();
    };
    saveButton.onPressed = [this] { handleSaveTriggerPressed(); };
    saveButton.onClick = [this] {
        if (suppressNextSaveButtonClick) {
            suppressNextSaveButtonClick = false;
            return;
        }

        showSavePopup();
    };

    for (auto* button : { &previousButton, &nextButton, &resetButton, &saveButton }) {
        button->setScaleMultiplier(theme.metrics.presetHeader.buttonScale);
        button->setIconScaleMultiplier(theme.metrics.presetHeader.buttonIconScaleMultiplier);
        addAndMakeVisible(button);
    }
    addAndMakeVisible(selectorButton);

    presetUiSnapshot = presetUiSnapshotSource.getPresetUiSnapshot();
    presetUiSnapshotSource.addPresetUiSnapshotListener(*this);
    updateButtonState();
}

PresetHeaderControlsComponent::~PresetHeaderControlsComponent() {
    dismissOpenPopup();
    presetUiSnapshotSource.removePresetUiSnapshotListener(*this);
}

int PresetHeaderControlsComponent::getPreferredWidth() const {
    const auto& metrics = theme.metrics.presetHeader;
    const auto buttonSide = previousButton.getPreferredSideLength();
    return buttonSide * 4
           + getTextBoxPreferredWidth()
           + metrics.displayGap * 2
           + metrics.groupGap
           + metrics.actionGap;
}

void PresetHeaderControlsComponent::resized() {
    const auto& metrics = theme.metrics.presetHeader;
    const auto buttonSide = previousButton.getPreferredSideLength();
    auto rowBounds = getLocalBounds();

    previousButton.setBounds(rowBounds.removeFromLeft(buttonSide));
    rowBounds.removeFromLeft(metrics.displayGap);
    selectorButton.setBounds(getScaledTextBoxBounds(rowBounds.removeFromLeft(getTextBoxPreferredWidth())));
    rowBounds.removeFromLeft(metrics.displayGap);
    nextButton.setBounds(rowBounds.removeFromLeft(buttonSide));
    rowBounds.removeFromLeft(metrics.groupGap);
    resetButton.setBounds(rowBounds.removeFromLeft(buttonSide));
    rowBounds.removeFromLeft(metrics.actionGap);
    saveButton.setBounds(rowBounds.removeFromLeft(buttonSide));
}

void PresetHeaderControlsComponent::presetUiSnapshotChanged(const PluginPresets::PresetUiSnapshot& snapshot) {
    if (presetUiSnapshot == snapshot)
        return;

    presetUiSnapshot = snapshot;
    updateButtonState();
}

void PresetHeaderControlsComponent::updateButtonState() {
    previousButton.setEnabled(presetUiSnapshot.canLoadPrevious);
    nextButton.setEnabled(presetUiSnapshot.canLoadNext);
    resetButton.setEnabled(presetUiSnapshot.canReset);
    saveButton.setEnabled(presetUiSnapshot.canSave);

    const auto shouldShowDirtyMarker
        = presetUiSnapshot.selectionStatus == PluginPresets::PresetSelectionStatus::selectedDirty;
    selectorButton.setDisplayText(presetUiSnapshot.selectedPresetName + (shouldShowDirtyMarker ? "*" : ""));

    previousButton.setActive(false);
    nextButton.setActive(false);
    resetButton.setActive(false);
    saveButton.setActive(false);
}

void PresetHeaderControlsComponent::dismissOpenPopup() {
    openPopup = OpenPopup::none;
    if (activeCallout != nullptr) {
        activeCallout->setLookAndFeel(nullptr);
        activeCallout->dismiss();
    }

    activeCallout = nullptr;
}

void PresetHeaderControlsComponent::launchCallout(std::unique_ptr<juce::Component> content,
                                                  const OpenPopup popupKind,
                                                  juce::Component& triggerComponent) {
    auto* parentComponent = getCalloutParentComponent();
    if (parentComponent == nullptr)
        return;

    auto* contentPtr = content.get();
    dismissOpenPopup();
    openPopup = popupKind;
    popupTriggerComponent = &triggerComponent;
    auto& callout = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           makeCalloutAnchorBounds(triggerComponent),
                                                           parentComponent);
    callout.setLookAndFeel(&popupLookAndFeel);
    callout.lookAndFeelChanged();
    callout.setDismissalMouseClicksAreAlwaysConsumed(false);
    activeCallout = &callout;
    focusPopupContent(contentPtr);
}

void PresetHeaderControlsComponent::showBrowserPopup() {
    if (getCalloutParentComponent() == nullptr)
        return;

    presetActions.refreshPresetCatalog();

    const auto safeThis = juce::Component::SafePointer<PresetHeaderControlsComponent>(this);
    auto content = std::make_unique<PresetBrowserPopupContent>(
        theme,
        presetUiSnapshot,
        PresetBrowserPopupContent::Callbacks{
            [safeThis](const PluginPresets::PresetId& presetId) {
                if (safeThis == nullptr)
                    return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

                return safeThis->presetActions.loadPreset(presetId);
            },
            [safeThis](const PluginPresets::PresetId& presetId) {
                if (safeThis == nullptr)
                    return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

                return safeThis->presetActions.deletePreset(presetId);
            },
            [safeThis] {
                if (safeThis == nullptr)
                    return;

                safeThis->handlePopupDismissed(OpenPopup::browser);
            }
        });
    content->setSize(content->getPreferredWidth(), content->getPreferredHeight());
    launchCallout(std::move(content), OpenPopup::browser, selectorButton);
}

void PresetHeaderControlsComponent::showSavePopup() {
    if (getCalloutParentComponent() == nullptr)
        return;

    presetActions.refreshPresetCatalog();

    const auto safeThis = juce::Component::SafePointer<PresetHeaderControlsComponent>(this);
    auto content = std::make_unique<PresetSavePopupContent>(
        theme,
        presetUiSnapshot,
        PresetSavePopupContent::Callbacks{
            [safeThis](const juce::String& name) {
                if (safeThis == nullptr)
                    return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::saveFailed);

                return safeThis->presetActions.savePresetAs(name);
            },
            [safeThis](const PluginPresets::PresetId& presetId, const juce::String& name) {
                if (safeThis == nullptr)
                    return PluginPresets::PresetActionResult::failed(
                        PluginPresets::PresetActionErrorCode::invalidOverwriteTarget);

                return safeThis->presetActions.overwritePreset(presetId, name);
            },
            [safeThis] {
                if (safeThis == nullptr)
                    return;

                safeThis->handlePopupDismissed(OpenPopup::save);
            }
        });
    content->setSize(content->getPreferredWidth(), content->getPreferredHeight());
    launchCallout(std::move(content), OpenPopup::save, saveButton);
}

void PresetHeaderControlsComponent::handleBrowserTriggerPressed() {
    if (openPopup != OpenPopup::browser)
        return;

    dismissOpenPopup();
    suppressNextSelectorClick = true;
}

void PresetHeaderControlsComponent::handleSaveTriggerPressed() {
    if (openPopup != OpenPopup::save)
        return;

    dismissOpenPopup();
    suppressNextSaveButtonClick = true;
}

void PresetHeaderControlsComponent::handlePopupDismissed(const OpenPopup popupKind) {
    if (popupKind == OpenPopup::browser)
        presetActions.refreshPresetCatalog();

    if (openPopup == popupKind)
        openPopup = OpenPopup::none;

    activeCallout = nullptr;
    const auto focusTarget = popupTriggerComponent;
    popupTriggerComponent = nullptr;
    if (focusTarget != nullptr)
        focusTarget->grabKeyboardFocus();
}

void PresetHeaderControlsComponent::focusPopupContent(juce::Component* content) const {
    if (content == nullptr)
        return;

    if (auto* savePopup = dynamic_cast<PresetSavePopupContent*>(content)) {
        savePopup->focusInitialControl();
        return;
    }

    content->grabKeyboardFocus();
}

juce::Component* PresetHeaderControlsComponent::getCalloutParentComponent() const {
    auto* parentComponent = getTopLevelComponent();
    if (parentComponent == nullptr)
        parentComponent = getParentComponent();

    return parentComponent;
}

juce::Rectangle<int> PresetHeaderControlsComponent::makeCalloutAnchorBounds(juce::Component& triggerComponent) const {
    auto* parentComponent = getCalloutParentComponent();
    if (parentComponent == nullptr)
        return {};

    auto anchorBounds = parentComponent->getLocalArea(this, triggerComponent.getBounds());
    return { anchorBounds.getCentreX(), anchorBounds.getBottom(), 1, 1 };
}

int PresetHeaderControlsComponent::getTextBoxPreferredWidth() const {
    const auto logicalBounds = Ui::getLogicalAssetBounds(Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::textBox),
                                                         theme.metrics.assets.rasterScale,
                                                         { 0, 0 });
    return juce::jmax(1, juce::roundToInt(static_cast<float>(logicalBounds.getWidth()) * theme.metrics.presetHeader.textBoxScale));
}

juce::Rectangle<int> PresetHeaderControlsComponent::getScaledTextBoxBounds(
    const juce::Rectangle<int> availableBounds) const {
    return Ui::getScaledAssetBoundsWithin(Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::textBox),
                                          theme.metrics.assets.rasterScale,
                                          availableBounds,
                                          theme.metrics.presetHeader.textBoxScale);
}
