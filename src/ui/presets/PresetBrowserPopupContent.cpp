#include "PresetBrowserPopupContent.h"

#include "PresetActionMessageFormatter.h"
#include "ui/theme/PopupChrome.h"

PresetBrowserPopupContent::PresetBrowserPopupContent(const Ui::Theme& themeToUse,
                                                     PluginPresets::PresetUiSnapshot uiSnapshotToUse,
                                                     Callbacks callbacksToUse)
    : theme(themeToUse),
      uiSnapshot(std::move(uiSnapshotToUse)),
      callbacks(std::move(callbacksToUse)),
      popupLookAndFeel(themeToUse),
      statusComponent(themeToUse) {
    addAndMakeVisible(viewport);
    addAndMakeVisible(statusComponent);
    viewport.setViewedComponent(&listContent, false);
    viewport.setScrollBarsShown(true, false);
    setWantsKeyboardFocus(true);
    rebuildRows();
}

PresetBrowserPopupContent::~PresetBrowserPopupContent() {
    dismissDeleteConfirmation();
    if (callbacks.onDismiss)
        callbacks.onDismiss();
}

int PresetBrowserPopupContent::getPreferredWidth() const {
    return theme.metrics.presetPopup.browserWidth;
}

int PresetBrowserPopupContent::getPreferredHeight() const {
    const auto& popupMetrics = theme.metrics.popup;
    const auto visibleRows = juce::jlimit(1,
                                          theme.metrics.presetPopup.maxVisibleRows,
                                          static_cast<int>(uiSnapshot.presets.size()));
    const auto statusHeight = statusComponent.getPreferredHeight();
    const auto statusGap = statusHeight > 0 ? theme.metrics.presetPopup.statusTopGap : 0;
    return static_cast<int>(popupMetrics.padding * 2
                            + popupMetrics.rowHeight * static_cast<float>(visibleRows)
                            + popupMetrics.rowGap * static_cast<float>(juce::jmax(0, visibleRows - 1))
                            + static_cast<float>(statusGap)
                            + static_cast<float>(statusHeight));
}

void PresetBrowserPopupContent::paint(juce::Graphics& g) {
    Ui::paintPopupShell(g, getLocalBounds().toFloat(), theme);
}

void PresetBrowserPopupContent::resized() {
    const auto& popupMetrics = theme.metrics.popup;
    auto bounds = getLocalBounds().reduced(static_cast<int>(popupMetrics.padding));
    const auto statusHeight = statusComponent.getPreferredHeight();
    if (statusHeight > 0) {
        auto statusBounds = bounds.removeFromBottom(statusHeight);
        bounds.removeFromBottom(theme.metrics.presetPopup.statusTopGap);
        statusComponent.setBounds(statusBounds);
    } else {
        statusComponent.setBounds({});
    }
    viewport.setBounds(bounds);

    const auto rowHeight = static_cast<int>(popupMetrics.rowHeight);
    const auto rowGap = static_cast<int>(popupMetrics.rowGap);
    const auto contentHeight = static_cast<int>(rows.size()) * rowHeight
                               + juce::jmax(0, static_cast<int>(rows.size()) - 1) * rowGap;
    listContent.setSize(bounds.getWidth(), contentHeight);

    auto rowBounds = juce::Rectangle<int>(listContent.getWidth(), rowHeight);
    for (auto& row : rows) {
        row->setBounds(rowBounds);
        rowBounds.translate(0, rowHeight + rowGap);
    }
}

bool PresetBrowserPopupContent::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::escapeKey) {
        if (auto* callout = findParentComponentOfClass<juce::CallOutBox>())
            callout->dismiss();
        return true;
    }

    if (key == juce::KeyPress::upKey) {
        moveKeyboardSelection(-1);
        return true;
    }

    if (key == juce::KeyPress::downKey) {
        moveKeyboardSelection(1);
        return true;
    }

    if (key == juce::KeyPress::returnKey || key == juce::KeyPress::spaceKey) {
        activateKeyboardSelection();
        return true;
    }

    return false;
}

void PresetBrowserPopupContent::rebuildRows() {
    rows.clear();
    listContent.removeAllChildren();
    rows.reserve(uiSnapshot.presets.size());

    for (const auto& descriptor : uiSnapshot.presets) {
        auto row = std::make_unique<PresetListRowComponent>(theme,
                                                            descriptor,
                                                            uiSnapshot.selectedPresetId == std::optional<PluginPresets::PresetId>(descriptor.id),
                                                            PresetListRowComponent::Callbacks{});
        auto* rowPtr = row.get();
        PresetListRowComponent::Callbacks rowCallbacks;
        rowCallbacks.onLoadRequested = [this](const PluginPresets::PresetId& presetId) {
            const auto result = callbacks.onLoadPreset ? callbacks.onLoadPreset(presetId)
                                                       : PluginPresets::PresetActionResult::ok();
            if (!result.succeeded) {
                setStatusMessage(Ui::Presets::makePresetActionMessage(result));
                return;
            }

            setStatusMessage({});
            if (auto* callout = findParentComponentOfClass<juce::CallOutBox>())
                callout->dismiss();
        };
        rowCallbacks.onDeleteRequested = [this, descriptor, rowPtr](const PluginPresets::PresetDescriptor&, juce::Rectangle<int> deleteBounds) {
            if (rowPtr == nullptr)
                return;

            launchDeleteConfirmation(descriptor, *rowPtr, deleteBounds);
        };
        row->setCallbacks(std::move(rowCallbacks));
        listContent.addAndMakeVisible(*row);
        rows.push_back(std::move(row));
    }

    if (rows.empty()) {
        keyboardSelectedRowIndex = -1;
    } else if (uiSnapshot.selectedPresetId.has_value()) {
        const auto iterator = std::find_if(uiSnapshot.presets.begin(), uiSnapshot.presets.end(),
                                           [this](const PluginPresets::PresetDescriptor& descriptor) {
                                               return uiSnapshot.selectedPresetId == std::optional<PluginPresets::PresetId>(descriptor.id);
                                           });
        keyboardSelectedRowIndex = iterator != uiSnapshot.presets.end()
                                       ? static_cast<int>(std::distance(uiSnapshot.presets.begin(), iterator))
                                       : 0;
    } else {
        keyboardSelectedRowIndex = 0;
    }

    updateKeyboardSelection();
}

void PresetBrowserPopupContent::updateKeyboardSelection() {
    for (int index = 0; index < static_cast<int>(rows.size()); ++index)
        rows[static_cast<size_t>(index)]->setKeyboardSelected(index == keyboardSelectedRowIndex);
}

void PresetBrowserPopupContent::moveKeyboardSelection(const int step) {
    if (rows.empty())
        return;

    if (keyboardSelectedRowIndex < 0)
        keyboardSelectedRowIndex = 0;
    else
        keyboardSelectedRowIndex = juce::jlimit(0,
                                                static_cast<int>(rows.size()) - 1,
                                                keyboardSelectedRowIndex + step);

    updateKeyboardSelection();
    if (const auto* row = rows[static_cast<size_t>(keyboardSelectedRowIndex)].get()) {
        const auto rowBounds = row->getBounds();
        const auto currentViewArea = viewport.getViewArea();
        auto targetY = currentViewArea.getY();

        if (rowBounds.getY() < currentViewArea.getY()) {
            targetY = rowBounds.getY();
        } else if (rowBounds.getBottom() > currentViewArea.getBottom()) {
            targetY = rowBounds.getBottom() - currentViewArea.getHeight();
        }

        viewport.setViewPosition(viewport.getViewPositionX(), juce::jmax(0, targetY));
    }
}

void PresetBrowserPopupContent::activateKeyboardSelection() {
    if (keyboardSelectedRowIndex < 0 || keyboardSelectedRowIndex >= static_cast<int>(uiSnapshot.presets.size()))
        return;

    const auto& descriptor = uiSnapshot.presets[static_cast<size_t>(keyboardSelectedRowIndex)];
    const auto result = callbacks.onLoadPreset ? callbacks.onLoadPreset(descriptor.id)
                                               : PluginPresets::PresetActionResult::ok();
    if (!result.succeeded) {
        setStatusMessage(Ui::Presets::makePresetActionMessage(result));
        return;
    }

    setStatusMessage({});
    if (auto* callout = findParentComponentOfClass<juce::CallOutBox>())
        callout->dismiss();
}

void PresetBrowserPopupContent::removePresetFromSnapshot(const PluginPresets::PresetId& presetId) {
    const auto iterator = std::remove_if(uiSnapshot.presets.begin(), uiSnapshot.presets.end(),
                                         [&presetId](const PluginPresets::PresetDescriptor& descriptor) {
                                             return descriptor.id == presetId;
                                         });
    if (iterator == uiSnapshot.presets.end())
        return;

    uiSnapshot.presets.erase(iterator, uiSnapshot.presets.end());
    if (uiSnapshot.selectedPresetId == std::optional<PluginPresets::PresetId>(presetId))
        uiSnapshot.selectedPresetId.reset();

    setSize(getPreferredWidth(), getPreferredHeight());
    rebuildRows();
    resized();
    repaint();
}

void PresetBrowserPopupContent::dismissDeleteConfirmation() {
    if (activeDeleteCallout == nullptr)
        return;

    activeDeleteCallout->setLookAndFeel(nullptr);
    activeDeleteCallout->dismiss();
    activeDeleteCallout = nullptr;
    activeDeletePresetId.reset();
}

void PresetBrowserPopupContent::launchDeleteConfirmation(const PluginPresets::PresetDescriptor& descriptor,
                                                         const PresetListRowComponent& row,
                                                         const juce::Rectangle<int> deleteBounds) {
    if (activeDeleteCallout != nullptr
        && activeDeletePresetId == std::optional<PluginPresets::PresetId>(descriptor.id)) {
        dismissDeleteConfirmation();
        return;
    }

    dismissDeleteConfirmation();

    auto* parentComponent = getTopLevelComponent();
    if (parentComponent == nullptr)
        parentComponent = getParentComponent();
    if (parentComponent == nullptr)
        return;

    const auto safeThis = juce::Component::SafePointer<PresetBrowserPopupContent>(this);
    auto content = std::make_unique<PresetDeleteConfirmPopupContent>(
        theme,
        descriptor.name,
        [safeThis, descriptor] {
            if (safeThis == nullptr)
                return PluginPresets::PresetActionResult::failed(PluginPresets::PresetActionErrorCode::presetNotFound);

            const auto result = safeThis->callbacks.onDeletePreset
                                    ? safeThis->callbacks.onDeletePreset(descriptor.id)
                                    : PluginPresets::PresetActionResult::ok();
            if (!result.succeeded)
                return result;

            safeThis->setStatusMessage({});
            safeThis->removePresetFromSnapshot(descriptor.id);
            return result;
        },
        [safeThis] {
            if (safeThis == nullptr)
                return;

            safeThis->activeDeleteCallout = nullptr;
            safeThis->activeDeletePresetId.reset();
        });
    content->setSize(content->getPreferredWidth(), content->getPreferredHeight());

    auto anchorBounds = parentComponent->getLocalArea(&row, deleteBounds);
    anchorBounds = {anchorBounds.getCentreX(), anchorBounds.getCentreY(), 1, 1};
    auto& callout = juce::CallOutBox::launchAsynchronously(std::move(content), anchorBounds, parentComponent);
    callout.setLookAndFeel(&popupLookAndFeel);
    callout.lookAndFeelChanged();
    callout.setDismissalMouseClicksAreAlwaysConsumed(false);
    activeDeleteCallout = &callout;
    activeDeletePresetId = descriptor.id;
    callout.grabKeyboardFocus();
}

void PresetBrowserPopupContent::setStatusMessage(juce::String newMessage) {
    statusComponent.setMessage(std::move(newMessage));
    setSize(getPreferredWidth(), getPreferredHeight());
    resized();
    repaint();
}
