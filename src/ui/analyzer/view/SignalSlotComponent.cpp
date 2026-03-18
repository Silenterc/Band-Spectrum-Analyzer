#include "SignalSlotComponent.h"

#include <algorithm>
#include <cmath>

#include "../../UiButtonDrawing.h"
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

    const auto modePickerWidth = std::ceil(modeWidth + slotMetrics.modePickerPaddingX * 2.0f);
    const auto topRowWidth = slotMetrics.sourceToggleWidth + slotMetrics.sectionGap + modePickerWidth;
    const auto bottomRowWidth = getActionClusterWidth();
    const auto totalWidth = slotMetrics.cellPaddingX + std::max(topRowWidth, bottomRowWidth) + slotMetrics.cellPaddingX;

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

    const auto drawToggleHalf = [this, &g, &slotMetrics](const juce::Rectangle<float> &halfBounds,
                                                         const bool active,
                                                         const bool hovered,
                                                         const bool enabled,
                                                         const juce::String &label,
                                                         const bool topHalf) {
        auto fill = active ? theme.accentButton : theme.controlSurface;
        if (hovered && enabled)
            fill = active ? theme.accentButton.brighter(0.14f) : theme.controlSurfaceHover;

        juce::Path halfPath;
        halfPath.addRoundedRectangle(halfBounds.getX(),
                                     halfBounds.getY(),
                                     halfBounds.getWidth(),
                                     halfBounds.getHeight(),
                                     slotMetrics.buttonCornerRadius,
                                     slotMetrics.buttonCornerRadius,
                                     topHalf,
                                     topHalf,
                                     !topHalf,
                                     !topHalf);
        g.setColour(enabled ? fill : fill.withMultipliedAlpha(0.45f));
        g.fillPath(halfPath);

        g.setColour(enabled ? theme.controlText : theme.subtleText.withMultipliedAlpha(0.75f));
        g.setFont(slotMetrics.hintFontHeight + 1.0f);
        g.drawText(label, halfBounds.toNearestInt(), juce::Justification::centred);
    };

    if (isDragged) {
        g.setColour(juce::Colours::black.withAlpha(0.22f));
        g.fillRoundedRectangle(bounds.translated(0.0f, slotMetrics.shadowOffsetY), slotMetrics.cellCornerRadius);
    }

    g.setColour(theme.controlSurface);
    g.fillRoundedRectangle(bounds, slotMetrics.cellCornerRadius);

    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), slotMetrics.cellCornerRadius, 1.0f);

    const auto sourceToggleBounds = getSourceToggleBounds();
    g.setColour(theme.controlSurface);
    g.fillRoundedRectangle(sourceToggleBounds, slotMetrics.buttonCornerRadius);
    drawToggleHalf(getSourceMainBounds(),
                   settings.configuration.source == Analyzer::SignalSource::main,
                   hoveredHitArea == SignalSlotHitArea::sourceToggleMain,
                   true,
                   "Main",
                   true);
    drawToggleHalf(getSourceSidechainBounds(),
                   settings.configuration.source == Analyzer::SignalSource::sidechain,
                   hoveredHitArea == SignalSlotHitArea::sourceToggleSidechain,
                   isSidechainRouted,
                   "Sidechain",
                   false);
    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(sourceToggleBounds.reduced(0.5f), slotMetrics.buttonCornerRadius, 1.0f);
    g.drawHorizontalLine(static_cast<int>(std::round(getSourceMainBounds().getBottom())),
                         sourceToggleBounds.getX() + 1.0f,
                         sourceToggleBounds.getRight() - 1.0f);

    drawActionButton(getModePickerBounds(),
                     hoveredHitArea == SignalSlotHitArea::modePicker,
                     Ui::getSignalModeLabel(settings.configuration.mode),
                     theme.controlSurface,
                     theme.controlText,
                     slotMetrics.titleFontHeight);

    const auto swatchBounds = getSwatchBounds();
    const auto signalColour = Ui::getSignalPresetColour(settings.colourIndex).withAlpha(settings.opacity);
    g.setColour(signalColour);
    g.fillRoundedRectangle(swatchBounds, slotMetrics.swatchCornerRadius);
    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(swatchBounds, slotMetrics.swatchCornerRadius, 1.0f);

    const auto dragHandleBounds = getDragHandleBounds();
    g.setColour(hoveredHitArea == SignalSlotHitArea::dragHandle ? theme.controlSurfaceHover : theme.controlSurface);
    g.fillRoundedRectangle(dragHandleBounds, slotMetrics.buttonCornerRadius);
    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(dragHandleBounds.reduced(0.5f), slotMetrics.buttonCornerRadius, 1.0f);
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
                                    : theme.controlSurface;
    drawActionButton(visibilityBounds,
                     visibilityHovered && !settings.visible,
                     settings.visible ? "On" : "Off",
                     visibilityFill,
                     theme.controlText,
                     11.0f);

    const auto freezeBounds = getFreezeBounds();
    const auto freezeHovered = hoveredHitArea == SignalSlotHitArea::freeze;
    const auto freezeStyle = Ui::getSnowflakeButtonStyle(theme, settings.frozen, freezeHovered);
    Ui::drawSnowflakeActionButton(g, freezeBounds, theme, freezeStyle, 4.0f);

    drawActionButton(getRemoveBounds(),
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
        case SignalSlotHitArea::sourceToggleMain:
            if (settings.configuration.source != Analyzer::SignalSource::main) {
                settings.configuration.source = Analyzer::SignalSource::main;
                if (onSignalSelected)
                    onSignalSelected(slotIndex, settings.configuration.source, settings.configuration.mode);
            }
            break;
        case SignalSlotHitArea::sourceToggleSidechain:
            if (isSidechainRouted && settings.configuration.source != Analyzer::SignalSource::sidechain) {
                settings.configuration.source = Analyzer::SignalSource::sidechain;
                if (onSignalSelected)
                    onSignalSelected(slotIndex, settings.configuration.source, settings.configuration.mode);
            }
            break;
        case SignalSlotHitArea::modePicker:
            showSignalMenu();
            break;
        case SignalSlotHitArea::swatch:
            showColourMenu();
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
        case SignalSlotHitArea::body:
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

juce::Rectangle<float> SignalSlotComponent::getContentBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    return getLocalBounds().toFloat().reduced(slotMetrics.cellPaddingX, slotMetrics.cellPaddingY);
}

juce::Rectangle<float> SignalSlotComponent::getTopRowBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto contentBounds = getContentBounds();
    contentBounds.setHeight(slotMetrics.topRowHeight);
    return contentBounds;
}

juce::Rectangle<float> SignalSlotComponent::getBottomRowBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto contentBounds = getContentBounds();
    const auto y = contentBounds.getBottom() - slotMetrics.actionSize;
    return {contentBounds.getX(), y, contentBounds.getWidth(), slotMetrics.actionSize};
}

juce::Rectangle<float> SignalSlotComponent::getSourceToggleBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto topRowBounds = getTopRowBounds();
    topRowBounds.setWidth(slotMetrics.sourceToggleWidth);
    return topRowBounds;
}

juce::Rectangle<float> SignalSlotComponent::getSourceMainBounds() const {
    auto bounds = getSourceToggleBounds();
    bounds.setHeight(bounds.getHeight() * 0.5f);
    return bounds;
}

juce::Rectangle<float> SignalSlotComponent::getSourceSidechainBounds() const {
    auto bounds = getSourceToggleBounds();
    bounds.removeFromTop(bounds.getHeight() * 0.5f);
    return bounds;
}

juce::Rectangle<float> SignalSlotComponent::getModePickerBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto topRowBounds = getTopRowBounds();
    topRowBounds.removeFromLeft(slotMetrics.sourceToggleWidth + slotMetrics.sectionGap);
    return topRowBounds;
}

juce::Rectangle<float> SignalSlotComponent::getSwatchBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto rowBounds = getBottomRowBounds();
    return {rowBounds.getX(), rowBounds.getY(), slotMetrics.actionSize, slotMetrics.actionSize};
}

juce::Rectangle<float> SignalSlotComponent::getDragHandleBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto swatchBounds = getSwatchBounds();
    const auto x = swatchBounds.getRight() + slotMetrics.actionGap;
    return {x, swatchBounds.getY(), slotMetrics.actionSize, slotMetrics.actionSize};
}

juce::Rectangle<float> SignalSlotComponent::getVisibilityBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto dragHandleBounds = getDragHandleBounds();
    const auto x = dragHandleBounds.getRight() + slotMetrics.actionGap;
    return {x, dragHandleBounds.getY(), slotMetrics.actionSize, slotMetrics.actionSize};
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
    return slotMetrics.actionSize * 5.0f + slotMetrics.actionGap * 4.0f;
}

SignalSlotHitArea SignalSlotComponent::getHitAreaAt(const juce::Point<float> &position) const {
    if (getSourceMainBounds().contains(position))
        return SignalSlotHitArea::sourceToggleMain;

    if (getSourceSidechainBounds().contains(position))
        return SignalSlotHitArea::sourceToggleSidechain;

    if (getModePickerBounds().contains(position))
        return SignalSlotHitArea::modePicker;

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
        case SignalSlotHitArea::modePicker:
            return OpenPopupMenu::mode;
        case SignalSlotHitArea::swatch:
            return OpenPopupMenu::colour;
        case SignalSlotHitArea::sourceToggleMain:
        case SignalSlotHitArea::sourceToggleSidechain:
        case SignalSlotHitArea::dragHandle:
        case SignalSlotHitArea::visibility:
        case SignalSlotHitArea::freeze:
        case SignalSlotHitArea::remove:
        case SignalSlotHitArea::body:
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

    auto anchorBounds = parentComponent->getLocalArea(this, getModePickerBounds().toNearestInt());
    anchorBounds = {anchorBounds.getRight(), anchorBounds.getY() - 2, 1, 1};
    launchCallout(std::move(content), OpenPopupMenu::mode, anchorBounds);
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

    if (getSourceMainBounds().contains(position)
        || (getSourceSidechainBounds().contains(position) && isSidechainRouted)
        || getModePickerBounds().contains(position)
        || getVisibilityBounds().contains(position)
        || getFreezeBounds().contains(position)
        || getRemoveBounds().contains(position)) {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        return;
    }

    setMouseCursor(juce::MouseCursor::NormalCursor);
}
