#include "PresetBrowserPopupContent.h"

#include "ui/presets/model/PresetActionMessageFormatter.h"
#include "ui/theme/PopupChrome.h"

PresetBrowserPopupContent::PresetBrowserPopupContent(const Ui::Theme& themeToUse,
                                                     Ui::Presets::PresetUiSnapshot uiSnapshotToUse,
                                                     Callbacks callbacksToUse)
    : theme(themeToUse),
      uiSnapshot(std::move(uiSnapshotToUse)),
      callbacks(std::move(callbacksToUse)),
      statusComponent(themeToUse) {
    jassert(callbacks.onLoadPreset != nullptr);
    jassert(callbacks.onDeleteConfirmationRequested != nullptr);
    jassert(callbacks.onCloseRequested != nullptr);
    addAndMakeVisible(viewport);
    addAndMakeVisible(statusComponent);
    viewport.setViewedComponent(&listContent, false);
    viewport.setScrollBarsShown(true, false);
    setWantsKeyboardFocus(true);
    rebuildRows();
}

PresetBrowserPopupContent::~PresetBrowserPopupContent() {
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

void PresetBrowserPopupContent::focusInitialControl() {
    grabKeyboardFocus();
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
        callbacks.onCloseRequested();
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
                                                            uiSnapshot.selectedPresetId == std::optional<Ui::Presets::PresetId>(descriptor.id),
                                                            PresetListRowComponent::Callbacks{});
        auto* rowPtr = row.get();
        PresetListRowComponent::Callbacks rowCallbacks;
        rowCallbacks.onLoadRequested = [this](const Ui::Presets::PresetId& presetId) {
            const auto result = callbacks.onLoadPreset(presetId);
            if (!result.succeeded) {
                setStatusMessage(Ui::Presets::makePresetActionMessage(result));
                return;
            }

            setStatusMessage({});
            callbacks.onCloseRequested();
        };
        rowCallbacks.onDeleteRequested = [this, descriptor, rowPtr](const Ui::Presets::PresetDescriptor&, juce::Rectangle<int> deleteBounds) {
            if (rowPtr == nullptr)
                return;

            callbacks.onDeleteConfirmationRequested(descriptor, *rowPtr, deleteBounds);
        };
        row->setCallbacks(std::move(rowCallbacks));
        listContent.addAndMakeVisible(*row);
        rows.push_back(std::move(row));
    }

    if (rows.empty()) {
        keyboardSelectedRowIndex = -1;
    } else if (uiSnapshot.selectedPresetId.has_value()) {
        const auto iterator = std::find_if(uiSnapshot.presets.begin(), uiSnapshot.presets.end(),
                                           [this](const Ui::Presets::PresetDescriptor& descriptor) {
                                               return uiSnapshot.selectedPresetId == std::optional<Ui::Presets::PresetId>(descriptor.id);
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
    const auto result = callbacks.onLoadPreset(descriptor.id);
    if (!result.succeeded) {
        setStatusMessage(Ui::Presets::makePresetActionMessage(result));
        return;
    }

    setStatusMessage({});
    callbacks.onCloseRequested();
}

void PresetBrowserPopupContent::removePresetFromSnapshot(const Ui::Presets::PresetId& presetId) {
    const auto iterator = std::remove_if(uiSnapshot.presets.begin(), uiSnapshot.presets.end(),
                                         [&presetId](const Ui::Presets::PresetDescriptor& descriptor) {
                                             return descriptor.id == presetId;
                                         });
    if (iterator == uiSnapshot.presets.end())
        return;

    uiSnapshot.presets.erase(iterator, uiSnapshot.presets.end());
    if (uiSnapshot.selectedPresetId == std::optional<Ui::Presets::PresetId>(presetId))
        uiSnapshot.selectedPresetId.reset();

    setSize(getPreferredWidth(), getPreferredHeight());
    rebuildRows();
    resized();
    repaint();
}

void PresetBrowserPopupContent::setStatusMessage(juce::String newMessage) {
    statusComponent.setMessage(std::move(newMessage));
    setSize(getPreferredWidth(), getPreferredHeight());
    resized();
    repaint();
}
