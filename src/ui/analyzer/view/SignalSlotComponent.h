#pragma once

#include <functional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../SignalSlotUiState.h"
#include "../../UiTheme.h"
#include "../model/SignalSlotOptions.h"

enum class SignalSlotHitArea {
    swatch,
    label,
    dragHandle,
    visibility,
    freeze,
    remove,
    body
};

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
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    void mouseDoubleClick(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;

private:
    enum class OpenPopupMenu {
        none,
        signal,
        colour
    };

    juce::Rectangle<float> getSwatchBounds() const;
    juce::Rectangle<float> getLabelBounds() const;
    juce::Rectangle<float> getDragHandleBounds() const;
    juce::Rectangle<float> getVisibilityBounds() const;
    juce::Rectangle<float> getFreezeBounds() const;
    juce::Rectangle<float> getRemoveBounds() const;
    float getActionClusterWidth() const;
    SignalSlotHitArea getHitAreaAt(const juce::Point<float> &position) const;
    bool isColourAvailable(int colourIndex) const;
    bool isSignalAvailable(Analyzer::SignalSource source, Analyzer::SignalMode mode) const;
    static OpenPopupMenu popupMenuForHitArea(SignalSlotHitArea hitArea);
    void dismissOpenMenu();
    void launchCallout(std::unique_ptr<juce::Component> content, OpenPopupMenu kind,
                       const juce::Rectangle<int> &anchorBounds);
    void showSignalMenu();
    void showColourMenu();
    void setHoveredHitArea(std::optional<SignalSlotHitArea> hitArea);
    void updateCursor(const juce::Point<float> &position);

    const Ui::Theme &theme;
    size_t slotIndex = 0;
    Ui::SignalSlotState settings;
    std::vector<int> usedColours;
    std::vector<Ui::SignalSlotKey> usedSignalConfigs;
    bool isSidechainRouted = false;
    SignalSlotHitArea mouseDownHitArea = SignalSlotHitArea::body;
    juce::Point<float> mouseDownPosition;
    bool isTrackingOpacityDrag = false;
    bool didOpacityDrag = false;
    bool isTrackingReorderDrag = false;
    bool isReorderDragging = false;
    bool isDragged = false;
    bool suppressMouseUpAction = false;
    std::optional<SignalSlotHitArea> hoveredHitArea;
    OpenPopupMenu openPopupMenu = OpenPopupMenu::none;
    juce::Component::SafePointer<juce::CallOutBox> activeCallout;
    float dragStartOpacity = Ui::defaultSignalOpacity;
};
