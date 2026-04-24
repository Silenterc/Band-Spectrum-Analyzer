#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "PresetPopupStatusComponent.h"
#include "PresetDeleteConfirmPopupContent.h"
#include "PresetListRowComponent.h"
#include "plugin/presets/PresetTypes.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/PopupLookAndFeel.h"

class PresetBrowserPopupContent final : public juce::Component {
public:
    struct Callbacks {
        std::function<PluginPresets::PresetActionResult(const PluginPresets::PresetId&)> onLoadPreset;
        std::function<PluginPresets::PresetActionResult(const PluginPresets::PresetId&)> onDeletePreset;
        std::function<void()> onDismiss;
    };

    PresetBrowserPopupContent(const Ui::Theme& themeToUse,
                              PluginPresets::PresetUiSnapshot uiSnapshotToUse,
                              Callbacks callbacksToUse);
    ~PresetBrowserPopupContent() override;

    [[nodiscard]] int getPreferredWidth() const;
    [[nodiscard]] int getPreferredHeight() const;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void rebuildRows();
    void updateKeyboardSelection();
    void moveKeyboardSelection(int step);
    void activateKeyboardSelection();
    void removePresetFromSnapshot(const PluginPresets::PresetId& presetId);
    void dismissDeleteConfirmation();
    void launchDeleteConfirmation(const PluginPresets::PresetDescriptor& descriptor,
                                  const PresetListRowComponent& row,
                                  juce::Rectangle<int> deleteBounds);
    void setStatusMessage(juce::String newMessage);

    const Ui::Theme& theme;
    PluginPresets::PresetUiSnapshot uiSnapshot;
    Callbacks callbacks;
    PopupLookAndFeel popupLookAndFeel;
    juce::Viewport viewport;
    juce::Component listContent;
    PresetPopupStatusComponent statusComponent;
    std::vector<std::unique_ptr<PresetListRowComponent>> rows;
    juce::Component::SafePointer<juce::CallOutBox> activeDeleteCallout;
    std::optional<PluginPresets::PresetId> activeDeletePresetId;
    int keyboardSelectedRowIndex = -1;
};
