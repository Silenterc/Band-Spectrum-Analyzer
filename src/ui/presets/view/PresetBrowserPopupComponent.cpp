#include "PresetBrowserPopupComponent.h"

PresetBrowserPopupComponent::PresetBrowserPopupComponent(const Ui::Theme& themeToUse,
                                                         Ui::Presets::PresetUiSnapshot uiSnapshotToUse,
                                                         Callbacks callbacksToUse)
    : theme(themeToUse),
      callbacks(std::move(callbacksToUse)),
      popupLookAndFeel(themeToUse),
      deleteCalloutPresenter(popupLookAndFeel),
      content(themeToUse,
              std::move(uiSnapshotToUse),
              PresetBrowserPopupContent::Callbacks{
                  callbacks.onLoadPreset,
                  [this](const Ui::Presets::PresetDescriptor& descriptor,
                         const PresetListRowComponent& row,
                         const juce::Rectangle<int> deleteBounds) {
                      launchDeleteConfirmation(descriptor, row, deleteBounds);
                  },
                  callbacks.onCloseRequested}) {
    jassert(callbacks.onLoadPreset != nullptr);
    jassert(callbacks.onDeletePreset != nullptr);
    jassert(callbacks.onCloseRequested != nullptr);
    jassert(callbacks.onDismissed != nullptr);
    addAndMakeVisible(content);
    setWantsKeyboardFocus(true);
}

PresetBrowserPopupComponent::~PresetBrowserPopupComponent() {
    dismissDeleteConfirmation();
    callbacks.onDismissed();
}

int PresetBrowserPopupComponent::getPreferredWidth() const {
    return content.getPreferredWidth();
}

int PresetBrowserPopupComponent::getPreferredHeight() const {
    return content.getPreferredHeight();
}

void PresetBrowserPopupComponent::focusInitialControl() {
    content.focusInitialControl();
}

void PresetBrowserPopupComponent::resized() {
    content.setBounds(getLocalBounds());
}

void PresetBrowserPopupComponent::dismissDeleteConfirmation() {
    if (!deleteCalloutPresenter.isShowing())
        return;

    deleteCalloutPresenter.dismiss();
    activeDeletePresetId.reset();
}

void PresetBrowserPopupComponent::launchDeleteConfirmation(const Ui::Presets::PresetDescriptor& descriptor,
                                                          const PresetListRowComponent& row,
                                                          const juce::Rectangle<int> deleteBounds) {
    if (deleteCalloutPresenter.isShowing()
        && activeDeletePresetId == std::optional<Ui::Presets::PresetId>(descriptor.id)) {
        dismissDeleteConfirmation();
        return;
    }

    dismissDeleteConfirmation();

    auto* parentComponent = Ui::findCalloutParentComponent(*this);
    if (parentComponent == nullptr)
        return;

    const auto safeThis = juce::Component::SafePointer<PresetBrowserPopupComponent>(this);
    auto deleteContent = std::make_unique<PresetDeleteConfirmPopupContent>(
        theme,
        descriptor.name,
        [safeThis, descriptor] {
            if (safeThis == nullptr)
                return Ui::Presets::PresetActionResult::failed(Ui::Presets::PresetActionErrorCode::presetNotFound);

            const auto result = safeThis->callbacks.onDeletePreset(descriptor.id);
            if (!result.succeeded)
                return result;

            safeThis->content.setStatusMessage({});
            safeThis->content.removePresetFromSnapshot(descriptor.id);
            return result;
        },
        [safeThis] {
            if (safeThis == nullptr)
                return;

            safeThis->dismissDeleteConfirmation();
        },
        [safeThis] {
            if (safeThis == nullptr)
                return;

            safeThis->deleteCalloutPresenter.forget();
            safeThis->activeDeletePresetId.reset();
        });
    deleteContent->setSize(deleteContent->getPreferredWidth(), deleteContent->getPreferredHeight());

    auto anchorBounds = parentComponent->getLocalArea(&row, deleteBounds);
    anchorBounds = {anchorBounds.getCentreX(), anchorBounds.getCentreY(), 1, 1};
    auto* callout = deleteCalloutPresenter.launch(std::move(deleteContent), anchorBounds, *parentComponent);
    activeDeletePresetId = descriptor.id;
    if (callout != nullptr)
        callout->grabKeyboardFocus();
}
