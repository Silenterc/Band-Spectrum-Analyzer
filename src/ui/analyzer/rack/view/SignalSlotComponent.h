#pragma once

#include <memory>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/state/SignalSlotUiState.h"
#include "ui/widgets/PopupLookAndFeel.h"
#include "ui/widgets/PadButton.h"
#include "ui/theme/UiTheme.h"
#include "ui/analyzer/rack/model/SignalSlotOptions.h"
#include "SignalSlotActionButton.h"
#include "SignalSlotModeButton.h"
#include "SignalSlotSourceToggle.h"
#include "SignalSlotSwatchButton.h"

class SignalSlotComponent final : public juce::Component {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void signalSlotSourceSelected(size_t slotIndex, Analyzer::SignalSource source) = 0;
        virtual void signalSlotModeSelected(size_t slotIndex, Analyzer::SignalMode mode) = 0;
        virtual void signalSlotColourSelected(size_t slotIndex, int colourIndex) = 0;
        virtual void signalSlotVisibilityChanged(size_t slotIndex, bool isVisible) = 0;
        virtual void signalSlotFrozenChanged(size_t slotIndex, bool isFrozen) = 0;
        virtual void signalSlotRemoveRequested(size_t slotIndex) = 0;
        virtual void signalSlotOpacityChanged(size_t slotIndex, float opacity) = 0;
        virtual void signalSlotReorderDragStarted(size_t slotIndex, float startMouseX) = 0;
        virtual void signalSlotReorderDragged(float xPosition) = 0;
        virtual void signalSlotReorderDragEnded(float xPosition) = 0;
    };

    explicit SignalSlotComponent(const Ui::Theme &themeToUse);
    ~SignalSlotComponent() override;

    void setListener(Listener *listenerToUse);
    void setSlot(size_t slotIndexToUse, const Ui::SignalSlotState &settingsToUse,
                 const std::vector<int> &usedColoursToUse,
                 const std::vector<Ui::SignalSlotKey> &usedSignalConfigsToUse);
    void setSidechainAvailable(bool isAvailable);
    void setReorderEnabled(bool shouldEnableReorder);
    void setDragged(bool isDraggedValue);
    bool getDragged() const;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    enum class OpenPopupMenu {
        none,
        mode,
        colour
    };

    juce::Rectangle<float> getModuleBounds() const;
    void rebuildCachedBackground();
    void refreshChildState();
    juce::Rectangle<float> getContentBounds() const;
    juce::Rectangle<float> getSourceToggleBounds() const;
    juce::Rectangle<float> getControlColumnBounds() const;
    juce::Rectangle<float> getControlStackBounds() const;
    float getActionRowHeight() const;
    juce::Rectangle<float> getModeRowBounds() const;
    juce::Rectangle<float> getActionRowBounds() const;
    bool isColourAvailable(int colourIndex) const;
    bool isSignalAvailable(Analyzer::SignalSource source, Analyzer::SignalMode mode) const;
    void dismissOpenMenu();
    void launchCallout(std::unique_ptr<juce::Component> content, OpenPopupMenu kind,
                       const juce::Rectangle<int> &anchorBounds);
    void showModeMenu();
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
    float getParentRelativeX(const juce::MouseEvent &event) const;

    const Ui::Theme &theme;
    Listener *listener = nullptr;
    size_t slotIndex = 0;
    Ui::SignalSlotState settings;
    std::vector<int> usedColours;
    std::vector<Ui::SignalSlotKey> usedSignalConfigs;
    bool isSidechainRouted = false;
    bool isDragged = false;
    bool reorderEnabled = false;
    bool trackingBackgroundDrag = false;
    bool suppressNextModeButtonClick = false;
    bool suppressNextSwatchClick = false;
    OpenPopupMenu openPopupMenu = OpenPopupMenu::none;
    juce::Component::SafePointer<juce::CallOutBox> activeCallout;
    juce::Image cachedBackground;
    juce::Point<float> mouseDownPosition;
    PopupLookAndFeel popupLookAndFeel;
    SignalSlotSourceToggle sourceToggle;
    SignalSlotModeButton modeButton;
    SignalSlotSwatchButton swatchButton;
    PadButton soloButton;
    PadButton visibilityButton;
    PadButton freezeButton;
    SignalSlotActionButton removeButton;
};
