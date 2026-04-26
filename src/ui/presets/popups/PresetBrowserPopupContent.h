#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "PresetPopupStatusComponent.h"
#include "ui/presets/state/PresetUiState.h"
#include "ui/presets/view/PresetListRowComponent.h"
#include "ui/theme/UiTheme.h"

class PresetBrowserPopupContent final : public juce::Component {
public:
    struct Callbacks {
        std::function<Ui::Presets::PresetActionResult(const Ui::Presets::PresetId&)> onLoadPreset;
        std::function<void(const Ui::Presets::PresetDescriptor&,
                           const PresetListRowComponent&,
                           juce::Rectangle<int>)> onDeleteConfirmationRequested;
        std::function<void()> onCloseRequested;
    };

    PresetBrowserPopupContent(const Ui::Theme& themeToUse,
                              Ui::Presets::PresetUiSnapshot uiSnapshotToUse,
                              Callbacks callbacksToUse);
    ~PresetBrowserPopupContent() override;

    [[nodiscard]] int getPreferredWidth() const;
    [[nodiscard]] int getPreferredHeight() const;
    void focusInitialControl();
    void removePresetFromSnapshot(const Ui::Presets::PresetId& presetId);
    void setStatusMessage(juce::String newMessage);

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void rebuildRows();
    void updateKeyboardSelection();
    void moveKeyboardSelection(int step);
    void activateKeyboardSelection();

    const Ui::Theme& theme;
    Ui::Presets::PresetUiSnapshot uiSnapshot;
    Callbacks callbacks;
    juce::Viewport viewport;
    juce::Component listContent;
    PresetPopupStatusComponent statusComponent;
    std::vector<std::unique_ptr<PresetListRowComponent>> rows;
    int keyboardSelectedRowIndex = -1;
};
