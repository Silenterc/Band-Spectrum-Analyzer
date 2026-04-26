#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/presets/state/PresetUiState.h"
#include "ui/theme/UiTheme.h"

class PresetListRowComponent final : public juce::Component {
public:
    struct Callbacks {
        std::function<void(const Ui::Presets::PresetId&)> onLoadRequested;
        std::function<void(const Ui::Presets::PresetDescriptor&, juce::Rectangle<int>)> onDeleteRequested;
    };

    PresetListRowComponent(const Ui::Theme& themeToUse,
                           Ui::Presets::PresetDescriptor descriptorToUse,
                           bool isCurrentPresetToUse,
                           Callbacks callbacksToUse);

    void setCallbacks(Callbacks callbacksToUse);
    void setKeyboardSelected(bool shouldBeSelected);
    [[nodiscard]] const Ui::Presets::PresetId& getPresetId() const noexcept { return descriptor.id; }
    [[nodiscard]] juce::Rectangle<int> getDeleteBounds() const noexcept { return deleteBounds; }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    const Ui::Theme& theme;
    Ui::Presets::PresetDescriptor descriptor;
    Callbacks callbacks;
    bool isCurrentPreset = false;
    bool isKeyboardSelected = false;
    bool isHovered = false;
    bool isDeleteHovered = false;
    juce::Rectangle<int> deleteBounds;
};
