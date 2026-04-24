#pragma once

#include <functional>
#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "PresetPopupStatusComponent.h"
#include "plugin/presets/PresetTypes.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/PopupActionButton.h"

class PresetSavePopupContent final : public juce::Component {
public:
    struct Callbacks {
        std::function<PluginPresets::PresetActionResult(const juce::String&)> onSaveAs;
        std::function<PluginPresets::PresetActionResult(const PluginPresets::PresetId&, const juce::String&)> onOverwrite;
        std::function<void()> onDismiss;
    };

    PresetSavePopupContent(const Ui::Theme& themeToUse,
                           PluginPresets::PresetUiSnapshot uiSnapshotToUse,
                           Callbacks callbacksToUse);
    ~PresetSavePopupContent() override;

    [[nodiscard]] int getPreferredWidth() const;
    [[nodiscard]] int getPreferredHeight() const;
    void focusInitialControl();

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void refreshPrimaryButtonState();
    void invokePrimaryAction();
    [[nodiscard]] std::optional<PluginPresets::PresetDescriptor> findMatchingPreset(const juce::String& name) const;
    [[nodiscard]] juce::String makeInitialName() const;
    void setStatusMessage(juce::String newMessage);

    const Ui::Theme& theme;
    PluginPresets::PresetUiSnapshot uiSnapshot;
    Callbacks callbacks;
    juce::TextEditor nameEditor;
    PopupActionButton cancelButton;
    PopupActionButton primaryButton;
    PresetPopupStatusComponent statusComponent;
    std::optional<PluginPresets::PresetId> pendingOverwriteId;
};
