#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "PresetPopupStatusComponent.h"
#include "plugin/presets/PresetTypes.h"
#include "ui/theme/UiTheme.h"
#include "ui/widgets/PopupActionButton.h"

class PresetDeleteConfirmPopupContent final : public juce::Component {
public:
    PresetDeleteConfirmPopupContent(const Ui::Theme& themeToUse,
                                    juce::String presetNameToUse,
                                    std::function<PluginPresets::PresetActionResult()> onConfirmToUse,
                                    std::function<void()> onDismissToUse);
    ~PresetDeleteConfirmPopupContent() override;

    [[nodiscard]] int getPreferredWidth() const;
    [[nodiscard]] int getPreferredHeight() const;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void confirmDelete();
    void setStatusMessage(juce::String newMessage);

    const Ui::Theme& theme;
    juce::String presetName;
    std::function<PluginPresets::PresetActionResult()> onConfirm;
    std::function<void()> onDismiss;
    PopupActionButton cancelButton;
    PopupActionButton deleteButton;
    PresetPopupStatusComponent statusComponent;
};
