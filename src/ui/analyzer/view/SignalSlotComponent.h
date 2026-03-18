#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../SignalSlotUiState.h"
#include "../../UiTheme.h"
#include "../model/SignalSlotOptions.h"
#include "SignalSlotActionButton.h"
#include "SignalSlotDragHandle.h"
#include "SignalSlotModeButton.h"
#include "SignalSlotSourceToggle.h"
#include "SignalSlotSwatchButton.h"

class SignalSlotComponent final : public juce::Component {
public:
    explicit SignalSlotComponent(const Ui::Theme &themeToUse);

    void setSlot(size_t slotIndexToUse, const Ui::SignalSlotState &settingsToUse,
                 const std::vector<int> &usedColoursToUse,
                 const std::vector<Ui::SignalSlotKey> &usedSignalConfigsToUse);
    void setSidechainAvailable(bool isAvailable);
    void setDragged(bool isDraggedValue);
    bool getDragged() const;
    size_t getSlotIndex() const;
    int getPreferredWidth() const;

    std::function<void(size_t, Analyzer::SignalSource, Analyzer::SignalMode)> onSignalSelected;
    std::function<void(size_t, int)> onColourSelected;
    std::function<void(size_t, bool)> onVisibilityChanged;
    std::function<void(size_t, bool)> onFrozenChanged;
    std::function<void(size_t)> onRemoveClicked;
    std::function<void(size_t, float)> onOpacityChanged;
    std::function<void(size_t, float)> onReorderDragStarted;
    std::function<void(size_t, float)> onReorderDragged;
    std::function<void(size_t, float)> onReorderDragEnded;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    enum class OpenPopupMenu {
        none,
        mode,
        colour
    };

    void refreshChildState();
    juce::Rectangle<float> getContentBounds() const;
    juce::Rectangle<float> getTopRowBounds() const;
    juce::Rectangle<float> getBottomRowBounds() const;
    float getActionClusterWidth() const;
    bool isColourAvailable(int colourIndex) const;
    bool isSignalAvailable(Analyzer::SignalSource source, Analyzer::SignalMode mode) const;
    void dismissOpenMenu();
    void launchCallout(std::unique_ptr<juce::Component> content, OpenPopupMenu kind,
                       const juce::Rectangle<int> &anchorBounds);
    void showSignalMenu();
    void showColourMenu();
    void handleSourceSelected(Analyzer::SignalSource source);
    void handleModePressed();
    void handleModeClicked();
    void handleVisibilityClicked();
    void handleFreezeClicked();
    void handleRemoveClicked();
    void handleSwatchClicked();
    void handleOpacityChanged(float opacity);
    void handleOpacityReset();
    void handleReorderDragStarted(float parentX);
    void handleReorderDragged(float parentX);
    void handleReorderDragEnded(float parentX);

    const Ui::Theme &theme;
    size_t slotIndex = 0;
    Ui::SignalSlotState settings;
    std::vector<int> usedColours;
    std::vector<Ui::SignalSlotKey> usedSignalConfigs;
    bool isSidechainRouted = false;
    bool isDragged = false;
    bool suppressNextModeButtonClick = false;
    OpenPopupMenu openPopupMenu = OpenPopupMenu::none;
    juce::Component::SafePointer<juce::CallOutBox> activeCallout;
    SignalSlotSourceToggle sourceToggle;
    SignalSlotModeButton modeButton;
    SignalSlotSwatchButton swatchButton;
    SignalSlotDragHandle dragHandle;
    SignalSlotActionButton visibilityButton;
    SignalSlotActionButton freezeButton;
    SignalSlotActionButton removeButton;
};
