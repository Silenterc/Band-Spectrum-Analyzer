#include "SignalSlotComponent.h"

#include <algorithm>
#include <cmath>

#include "../../UiButtonDrawing.h"
#include "../../UiIcons.h"
#include "../popups/SignalColourPopupContent.h"
#include "../popups/SignalSelectionPopupContent.h"

namespace {
    float measureTextWidth(const float fontHeight, const juce::String &text) {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(juce::Font(juce::FontOptions(fontHeight)), text, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, -1, true).getWidth();
    }
}

SignalSlotComponent::SignalSlotComponent(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
}

void SignalSlotComponent::setSlot(const size_t slotIndexToUse,
                                  const Ui::SignalSlotState &settingsToUse,
                                  const std::vector<int> &usedColoursToUse,
                                  const std::vector<Ui::SignalSlotKey> &usedSignalConfigsToUse) {
    slotIndex = slotIndexToUse;
    settings = settingsToUse;
    usedColours = usedColoursToUse;
    usedSignalConfigs = usedSignalConfigsToUse;
    repaint();
}

void SignalSlotComponent::setSidechainAvailable(const bool isAvailable) {
    isSidechainRouted = isAvailable;
}

void SignalSlotComponent::setDragged(const bool isDraggedValue) {
    isDragged = isDraggedValue;
    repaint();
}

bool SignalSlotComponent::getDragged() const {
    return isDragged;
}

size_t SignalSlotComponent::getSlotIndex() const {
    return slotIndex;
}

int SignalSlotComponent::getPreferredWidth() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto modeWidth = 0.0f;
    for (const auto &option: Ui::signalSlotOptions)
        modeWidth = std::max(modeWidth, measureTextWidth(slotMetrics.titleFontHeight, option.label));

    const auto hintWidth = std::max(measureTextWidth(slotMetrics.hintFontHeight, Ui::getSignalSourceHint(Analyzer::SignalSource::main)),
                                    measureTextWidth(slotMetrics.hintFontHeight, Ui::getSignalSourceHint(Analyzer::SignalSource::sidechain)));
    const auto textWidth = std::ceil(std::max(modeWidth, hintWidth));
    const auto actionWidth = getActionClusterWidth();

    const auto totalWidth = slotMetrics.cellPaddingX + slotMetrics.swatchSize + slotMetrics.sectionGap + textWidth
                            + slotMetrics.sectionGap + actionWidth + slotMetrics.cellPaddingX;

    return static_cast<int>(std::ceil(totalWidth));
}

void SignalSlotComponent::paint(juce::Graphics &g) {
    const auto &slotMetrics = theme.metrics.slot;
    const auto bounds = getLocalBounds().toFloat();
    const auto drawActionButton = [this, &g, &slotMetrics](const juce::Rectangle<float> &buttonBounds,
                                                           const bool isHovered,
                                                           const juce::String &label,
                                                           const juce::Colour &fill,
                                                           const juce::Colour &textColour,
                                                           const float fontHeight) {
        g.setColour(isHovered ? theme.controlSurfaceHover : fill);
        g.fillRoundedRectangle(buttonBounds, slotMetrics.buttonCornerRadius);
        g.setColour(textColour);
        g.setFont(fontHeight);
        g.drawText(label, buttonBounds.toNearestInt(), juce::Justification::centred);
    };
    if (isDragged) {
        g.setColour(juce::Colours::black.withAlpha(0.22f));
        g.fillRoundedRectangle(bounds.translated(0.0f, slotMetrics.shadowOffsetY), slotMetrics.cellCornerRadius);
    }

    g.setColour(theme.controlSurface);
    g.fillRoundedRectangle(bounds, slotMetrics.cellCornerRadius);

    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), slotMetrics.cellCornerRadius, 1.0f);

    const auto swatchBounds = getSwatchBounds();
    const auto signalColour = Ui::getSignalPresetColour(settings.colourIndex).withAlpha(settings.opacity);
    g.setColour(signalColour);
    g.fillRoundedRectangle(swatchBounds, slotMetrics.swatchCornerRadius);
    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(swatchBounds, slotMetrics.swatchCornerRadius, 1.0f);

    auto labelBounds = getLabelBounds();
    g.setColour(theme.controlText);
    g.setFont(slotMetrics.titleFontHeight);
    g.drawText(Ui::getSignalModeLabel(settings.configuration.mode), labelBounds.removeFromTop(static_cast<int>(slotMetrics.titleHeight)).toNearestInt(),
               juce::Justification::centredLeft);
    labelBounds.removeFromTop(static_cast<int>(slotMetrics.textStackGap));

    g.setColour(theme.subtleText);
    g.setFont(slotMetrics.hintFontHeight);
    g.drawText(Ui::getSignalSourceHint(settings.configuration.source), labelBounds.removeFromTop(static_cast<int>(slotMetrics.hintHeight)).toNearestInt(),
               juce::Justification::centredLeft);

    const auto dragHandleBounds = getDragHandleBounds();
    g.setColour(theme.subtleText);
    const auto gripX = dragHandleBounds.getCentreX() - slotMetrics.gripWidth * 0.5f;
    const auto gripY = dragHandleBounds.getCentreY() - slotMetrics.gripHeight * 0.5f;
    const auto columnStep = slotMetrics.gripWidth - slotMetrics.gripDotDiameter;
    const auto rowStep = (slotMetrics.gripHeight - slotMetrics.gripDotDiameter) * 0.5f;
    for (int dotColumn = 0; dotColumn < 2; ++dotColumn) {
        for (int dotRow = 0; dotRow < 3; ++dotRow) {
            const auto x = gripX + static_cast<float>(dotColumn) * columnStep;
            const auto y = gripY + static_cast<float>(dotRow) * rowStep;
            g.fillEllipse(x, y, slotMetrics.gripDotDiameter, slotMetrics.gripDotDiameter);
        }
    }

    const auto visibilityBounds = getVisibilityBounds();
    const auto visibilityHovered = hoveredHitArea == SignalSlotHitArea::visibility;
    const auto visibilityFill = settings.visible
                                    ? (visibilityHovered ? theme.accentButton.brighter(0.14f) : theme.accentButton)
                                    : (visibilityHovered ? theme.controlSurfaceHover : theme.controlSurface);
    drawActionButton(visibilityBounds,
                     false,
                     settings.visible ? "On" : "Off",
                     visibilityFill,
                     theme.controlText,
                     11.0f);

    const auto freezeBounds = getFreezeBounds();
    const auto freezeHovered = hoveredHitArea == SignalSlotHitArea::freeze;
    const auto freezeStyle = Ui::getSnowflakeButtonStyle(theme, settings.frozen, freezeHovered);
    Ui::drawSnowflakeActionButton(g,
                                  freezeBounds,
                                  theme,
                                  freezeStyle,
                                  4.0f);

    const auto removeBounds = getRemoveBounds();
    drawActionButton(removeBounds,
                     hoveredHitArea == SignalSlotHitArea::remove,
                     "x",
                     theme.controlSurface,
                     hoveredHitArea == SignalSlotHitArea::remove ? theme.controlText : theme.subtleText,
                     15.0f);
}

void SignalSlotComponent::mouseDown(const juce::MouseEvent &event) {
    mouseDownHitArea = getHitAreaAt(event.position);
    mouseDownPosition = event.position;
    suppressMouseUpAction = false;

    const auto requestedPopupMenu = popupMenuForHitArea(mouseDownHitArea);
    if (openPopupMenu != OpenPopupMenu::none) {
        const auto clickedSamePopupActivator = requestedPopupMenu != OpenPopupMenu::none
                                               && requestedPopupMenu == openPopupMenu;
        dismissOpenMenu();
        suppressMouseUpAction = clickedSamePopupActivator;
    }

    isTrackingOpacityDrag = mouseDownHitArea == SignalSlotHitArea::swatch;
    didOpacityDrag = false;
    isTrackingReorderDrag = mouseDownHitArea == SignalSlotHitArea::dragHandle;
    isReorderDragging = false;
    dragStartOpacity = settings.opacity;
}

void SignalSlotComponent::mouseMove(const juce::MouseEvent &event) {
    setHoveredHitArea(getHitAreaAt(event.position));
    updateCursor(event.position);
}

void SignalSlotComponent::mouseDrag(const juce::MouseEvent &event) {
    const auto delta = event.position - mouseDownPosition;

    if (isTrackingOpacityDrag) {
        if (std::abs(delta.y) > 3.0f && std::abs(delta.y) >= std::abs(delta.x))
            didOpacityDrag = true;

        if (didOpacityDrag) {
            const auto newOpacity = juce::jlimit(0.15f, 1.0f,
                                                 dragStartOpacity + (mouseDownPosition.y - event.position.y) * 0.005f);
            settings.opacity = newOpacity;
            if (onOpacityChanged)
                onOpacityChanged(slotIndex, newOpacity);
        }
    }

    if (isTrackingReorderDrag) {
        if (!isReorderDragging && delta.getDistanceFromOrigin() > 6.0f) {
            isReorderDragging = true;
            if (onReorderDragStarted)
                onReorderDragStarted(slotIndex, event.getEventRelativeTo(getParentComponent()).position.x);
        }

        if (isReorderDragging) {
            if (onReorderDragged)
                onReorderDragged(slotIndex, event.getEventRelativeTo(getParentComponent()).position.x);
        }
    }

    updateCursor(event.position);
    setHoveredHitArea(getHitAreaAt(event.position));
}

void SignalSlotComponent::mouseUp(const juce::MouseEvent &event) {
    const auto releaseHitArea = getHitAreaAt(event.position);

    if (suppressMouseUpAction) {
        suppressMouseUpAction = false;
        isTrackingOpacityDrag = false;
        didOpacityDrag = false;
        isTrackingReorderDrag = false;
        isReorderDragging = false;
        return;
    }

    if (isTrackingReorderDrag) {
        if (isReorderDragging) {
            if (onReorderDragEnded)
                onReorderDragEnded(slotIndex, event.getEventRelativeTo(getParentComponent()).position.x);
        }

        isTrackingReorderDrag = false;
        isReorderDragging = false;
        return;
    }

    if (isTrackingOpacityDrag) {
        if (!didOpacityDrag && releaseHitArea == SignalSlotHitArea::swatch)
            showColourMenu();

        isTrackingOpacityDrag = false;
        didOpacityDrag = false;
        return;
    }

    if (releaseHitArea != mouseDownHitArea)
        return;

    switch (releaseHitArea) {
        case SignalSlotHitArea::swatch:
            showColourMenu();
            break;
        case SignalSlotHitArea::label:
        case SignalSlotHitArea::body:
            showSignalMenu();
            break;
        case SignalSlotHitArea::visibility:
            settings.visible = !settings.visible;
            if (onVisibilityChanged)
                onVisibilityChanged(slotIndex, settings.visible);
            break;
        case SignalSlotHitArea::freeze:
            settings.frozen = !settings.frozen;
            if (onFrozenChanged)
                onFrozenChanged(slotIndex, settings.frozen);
            break;
        case SignalSlotHitArea::remove:
            if (onRemoveClicked)
                onRemoveClicked(slotIndex);
            break;
        case SignalSlotHitArea::dragHandle:
            break;
    }
}

void SignalSlotComponent::mouseDoubleClick(const juce::MouseEvent &event) {
    if (!getSwatchBounds().contains(event.position))
        return;

    settings.opacity = Ui::defaultSignalOpacity;
    if (onOpacityChanged)
        onOpacityChanged(slotIndex, settings.opacity);
}

void SignalSlotComponent::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    setHoveredHitArea(std::nullopt);
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

juce::Rectangle<float> SignalSlotComponent::getSwatchBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto contentBounds = getLocalBounds().toFloat().reduced(slotMetrics.cellPaddingX, slotMetrics.cellPaddingY);
    const auto y = contentBounds.getCentreY() - slotMetrics.swatchSize * 0.5f;
    return {contentBounds.getX(), y, slotMetrics.swatchSize, slotMetrics.swatchSize};
}

juce::Rectangle<float> SignalSlotComponent::getLabelBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto contentBounds = getLocalBounds().toFloat().reduced(slotMetrics.cellPaddingX, slotMetrics.cellPaddingY);
    contentBounds.removeFromLeft(slotMetrics.swatchSize + slotMetrics.sectionGap);
    contentBounds.removeFromRight(getActionClusterWidth() + slotMetrics.sectionGap);
    const auto textHeight = slotMetrics.titleHeight + slotMetrics.textStackGap + slotMetrics.hintHeight;
    const auto y = contentBounds.getCentreY() - textHeight * 0.5f;
    return {contentBounds.getX(), y, contentBounds.getWidth(), textHeight};
}

juce::Rectangle<float> SignalSlotComponent::getDragHandleBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto contentBounds = getLocalBounds().toFloat().reduced(slotMetrics.cellPaddingX, slotMetrics.cellPaddingY);
    const auto actionsWidth = getActionClusterWidth();
    const auto x = contentBounds.getRight() - actionsWidth;
    const auto y = contentBounds.getCentreY() - slotMetrics.actionSize * 0.5f;
    return {x, y, slotMetrics.actionSize, slotMetrics.actionSize};
}

juce::Rectangle<float> SignalSlotComponent::getVisibilityBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto dragHandleBounds = getDragHandleBounds();
    const auto x = dragHandleBounds.getRight() + slotMetrics.actionGap;
    const auto y = dragHandleBounds.getCentreY() - slotMetrics.actionSize * 0.5f;
    return {x, y, slotMetrics.actionSize, slotMetrics.actionSize};
}

juce::Rectangle<float> SignalSlotComponent::getFreezeBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto visibilityBounds = getVisibilityBounds();
    const auto x = visibilityBounds.getRight() + slotMetrics.actionGap;
    return {x, visibilityBounds.getY(), slotMetrics.actionSize, slotMetrics.actionSize};
}

juce::Rectangle<float> SignalSlotComponent::getRemoveBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto freezeBounds = getFreezeBounds();
    const auto x = freezeBounds.getRight() + slotMetrics.actionGap;
    return {x, freezeBounds.getY(), slotMetrics.actionSize, slotMetrics.actionSize};
}

float SignalSlotComponent::getActionClusterWidth() const {
    const auto &slotMetrics = theme.metrics.slot;
    return slotMetrics.actionSize * 4.0f + slotMetrics.actionGap * 3.0f;
}

SignalSlotHitArea SignalSlotComponent::getHitAreaAt(const juce::Point<float> &position) const {
    if (getSwatchBounds().contains(position))
        return SignalSlotHitArea::swatch;

    if (getDragHandleBounds().contains(position))
        return SignalSlotHitArea::dragHandle;

    if (getVisibilityBounds().contains(position))
        return SignalSlotHitArea::visibility;

    if (getFreezeBounds().contains(position))
        return SignalSlotHitArea::freeze;

    if (getRemoveBounds().contains(position))
        return SignalSlotHitArea::remove;

    if (getLabelBounds().contains(position))
        return SignalSlotHitArea::label;

    return SignalSlotHitArea::body;
}

bool SignalSlotComponent::isColourAvailable(const int colourIndex) const {
    if (colourIndex == settings.colourIndex)
        return true;

    return std::find(usedColours.begin(), usedColours.end(), colourIndex) == usedColours.end();
}

bool SignalSlotComponent::isSignalAvailable(const Analyzer::SignalSource source, const Analyzer::SignalMode mode) const {
    if (settings.configuration.source == source && settings.configuration.mode == mode)
        return true;

    return std::find(usedSignalConfigs.begin(), usedSignalConfigs.end(), std::make_pair(source, mode))
           == usedSignalConfigs.end();
}

SignalSlotComponent::OpenPopupMenu SignalSlotComponent::popupMenuForHitArea(const SignalSlotHitArea hitArea) {
    switch (hitArea) {
        case SignalSlotHitArea::swatch:
            return OpenPopupMenu::colour;
        case SignalSlotHitArea::label:
        case SignalSlotHitArea::body:
            return OpenPopupMenu::signal;
        case SignalSlotHitArea::dragHandle:
        case SignalSlotHitArea::visibility:
        case SignalSlotHitArea::freeze:
        case SignalSlotHitArea::remove:
            return OpenPopupMenu::none;
    }

    return OpenPopupMenu::none;
}

void SignalSlotComponent::dismissOpenMenu() {
    if (openPopupMenu == OpenPopupMenu::none)
        return;

    openPopupMenu = OpenPopupMenu::none;
    if (activeCallout != nullptr)
        activeCallout->dismiss();

    activeCallout = nullptr;
}

void SignalSlotComponent::launchCallout(std::unique_ptr<juce::Component> content,
                                        const OpenPopupMenu kind,
                                        const juce::Rectangle<int> &anchorBounds) {
    auto *parentComponent = getTopLevelComponent();
    if (parentComponent == nullptr)
        parentComponent = getParentComponent();

    if (parentComponent == nullptr)
        return;

    openPopupMenu = kind;
    auto &callout = juce::CallOutBox::launchAsynchronously(std::move(content), anchorBounds, parentComponent);
    callout.setDismissalMouseClicksAreAlwaysConsumed(false);
    activeCallout = &callout;
}

void SignalSlotComponent::showSignalMenu() {
    auto *parentComponent = getTopLevelComponent();
    if (parentComponent == nullptr)
        parentComponent = getParentComponent();

    if (parentComponent == nullptr)
        return;

    const auto safeThis = juce::Component::SafePointer<SignalSlotComponent>(this);
    auto content = std::make_unique<SignalSelectionPopupContent>(
        theme,
        isSidechainRouted,
        settings.configuration.source,
        settings.configuration.mode,
        [safeThis](const Analyzer::SignalSource source, const Analyzer::SignalMode mode) {
            if (safeThis == nullptr || !safeThis->onSignalSelected)
                return;

            safeThis->onSignalSelected(safeThis->slotIndex, source, mode);
        },
        [safeThis] {
            if (safeThis == nullptr)
                return;

            safeThis->openPopupMenu = OpenPopupMenu::none;
            safeThis->activeCallout = nullptr;
        });

    content->setAvailability([this](const Analyzer::SignalSource source, const Analyzer::SignalMode mode) {
        return isSignalAvailable(source, mode);
    });
    content->setSize(content->getPreferredWidth(), content->getPreferredHeight());

    auto anchorBounds = parentComponent->getLocalArea(this, getLabelBounds().toNearestInt());
    anchorBounds = {anchorBounds.getRight(), anchorBounds.getY() - 2, 1, 1};
    launchCallout(std::move(content), OpenPopupMenu::signal, anchorBounds);
}

void SignalSlotComponent::showColourMenu() {
    auto *parentComponent = getTopLevelComponent();
    if (parentComponent == nullptr)
        parentComponent = getParentComponent();

    if (parentComponent == nullptr)
        return;

    const auto safeThis = juce::Component::SafePointer<SignalSlotComponent>(this);
    auto content = std::make_unique<SignalColourPopupContent>(
        theme,
        [safeThis](const int selectedColourIndex) {
            if (safeThis == nullptr || !safeThis->onColourSelected)
                return;

            safeThis->onColourSelected(safeThis->slotIndex, selectedColourIndex);
        },
        [safeThis] {
            if (safeThis == nullptr)
                return;

            safeThis->openPopupMenu = OpenPopupMenu::none;
            safeThis->activeCallout = nullptr;
        });

    for (int colourIndex = 0; colourIndex < Ui::signalPresetCount; ++colourIndex) {
        content->addColourButton(Ui::getSignalPresetColour(colourIndex),
                                 colourIndex == settings.colourIndex,
                                 isColourAvailable(colourIndex),
                                 colourIndex);
    }

    content->setSize(content->getPreferredWidth(), content->getPreferredHeight());

    auto anchorBounds = parentComponent->getLocalArea(this, getSwatchBounds().toNearestInt());
    anchorBounds = {anchorBounds.getCentreX(), anchorBounds.getY() - 2, 1, 1};
    launchCallout(std::move(content), OpenPopupMenu::colour, anchorBounds);
}

void SignalSlotComponent::setHoveredHitArea(const std::optional<SignalSlotHitArea> hitArea) {
    if (hoveredHitArea == hitArea)
        return;

    hoveredHitArea = hitArea;
    repaint();
}

void SignalSlotComponent::updateCursor(const juce::Point<float> &position) {
    if (getSwatchBounds().contains(position)) {
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        return;
    }

    if (getDragHandleBounds().contains(position) || isReorderDragging) {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        return;
    }

    if (getLabelBounds().contains(position)
        || getVisibilityBounds().contains(position)
        || getFreezeBounds().contains(position)
        || getRemoveBounds().contains(position)) {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        return;
    }

    setMouseCursor(juce::MouseCursor::NormalCursor);
}
