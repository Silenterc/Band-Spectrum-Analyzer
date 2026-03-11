#include "SignalSlotComponent.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
    float measureTextWidth(const float fontHeight, const juce::String &text) {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(juce::Font(juce::FontOptions(fontHeight)), text, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, -1, true).getWidth();
    }

    class SignalSelectionRowButton final : public juce::Button {
    public:
        SignalSelectionRowButton(const Ui::Theme &themeToUse,
                                 const juce::String &labelToUse,
                                 const bool selectedToUse,
                                 const Analyzer::SignalSource sourceToUse,
                                 const Analyzer::SignalMode modeToUse)
            : juce::Button({}),
              source(sourceToUse),
              mode(modeToUse),
              theme(themeToUse),
              label(labelToUse),
              selected(selectedToUse) {
        }

        void paintButton(juce::Graphics &g, bool isMouseOverButton, bool) override {
            auto bounds = getLocalBounds().toFloat();
            const auto fill = selected ? theme.controlSurfaceHover.brighter(0.18f)
                                       : isMouseOverButton ? theme.controlSurfaceHover
                                                           : theme.controlSurface;
            g.setColour(isEnabled() ? fill : fill.withMultipliedAlpha(0.45f));
            g.fillRoundedRectangle(bounds, theme.metrics.slot.buttonCornerRadius);

            if (selected) {
                g.setColour(theme.controlBorder.brighter(0.6f));
                g.drawRoundedRectangle(bounds.reduced(0.5f), theme.metrics.slot.buttonCornerRadius, 1.5f);
            }

            g.setColour(isEnabled() ? theme.controlText : theme.subtleText.withMultipliedAlpha(0.75f));
            g.setFont(14.0f);
            g.drawText(label, bounds.reduced(10.0f, 0.0f).toNearestInt(), juce::Justification::centredLeft);
        }

        Analyzer::SignalSource source;
        Analyzer::SignalMode mode;

    private:
        const Ui::Theme &theme;
        juce::String label;
        bool selected = false;
    };

    class SignalSelectionCalloutContent final : public juce::Component {
    public:
        SignalSelectionCalloutContent(const Ui::Theme &themeToUse,
                                      const bool sidechainAvailableToUse,
                                      const Analyzer::SignalSource currentSourceToUse,
                                      const Analyzer::SignalMode currentModeToUse,
                                      std::function<void(Analyzer::SignalSource, Analyzer::SignalMode)> onSelectToUse,
                                      std::function<void()> onDismissToUse)
            : theme(themeToUse),
              sidechainAvailable(sidechainAvailableToUse),
              onSelect(std::move(onSelectToUse)),
              onDismiss(std::move(onDismissToUse)) {
            addAndMakeVisible(mainLabel);
            mainLabel.setText("Main", juce::dontSendNotification);
            mainLabel.setFont(juce::FontOptions(11.0f));
            mainLabel.setColour(juce::Label::textColourId, theme.subtleText);
            mainLabel.setJustificationType(juce::Justification::centredLeft);

            addAndMakeVisible(sidechainLabel);
            sidechainLabel.setText("Sidechain", juce::dontSendNotification);
            sidechainLabel.setFont(juce::FontOptions(11.0f));
            sidechainLabel.setColour(juce::Label::textColourId, theme.subtleText);
            sidechainLabel.setJustificationType(juce::Justification::centredLeft);

            auto makeButton = [this, currentSourceToUse, currentModeToUse](
                                  const juce::String &label,
                                  const Analyzer::SignalSource source,
                                  const Analyzer::SignalMode mode) {
                auto button = std::make_unique<SignalSelectionRowButton>(
                    theme, label, currentSourceToUse == source && currentModeToUse == mode, source, mode);
                auto *buttonPtr = button.get();
                addAndMakeVisible(*button);
                button->onClick = [this, buttonPtr] {
                    if (!buttonPtr->isEnabled())
                        return;

                    if (onSelect)
                        onSelect(buttonPtr->source, buttonPtr->mode);

                    if (auto *callout = findParentComponentOfClass<juce::CallOutBox>())
                        callout->dismiss();
                };
                return button;
            };

            buttons[0] = makeButton("Mid", Analyzer::SignalSource::main, Analyzer::SignalMode::mid);
            buttons[1] = makeButton("Side", Analyzer::SignalSource::main, Analyzer::SignalMode::side);
            buttons[2] = makeButton("Stereo", Analyzer::SignalSource::main, Analyzer::SignalMode::stereo);
            buttons[3] = makeButton("Mid", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::mid);
            buttons[4] = makeButton("Side", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::side);
            buttons[5] = makeButton("Stereo", Analyzer::SignalSource::sidechain, Analyzer::SignalMode::stereo);
        }

        ~SignalSelectionCalloutContent() override {
            if (onDismiss)
                onDismiss();
        }

        void setAvailability(const std::function<bool(Analyzer::SignalSource, Analyzer::SignalMode)> &isAvailable) {
            for (auto &button: buttons) {
                const auto isSidechainButton = button->source == Analyzer::SignalSource::sidechain;
                button->setVisible(sidechainAvailable || !isSidechainButton);
                button->setEnabled(isAvailable(button->source, button->mode));
            }

            mainLabel.setVisible(sidechainAvailable);
            sidechainLabel.setVisible(sidechainAvailable);
        }

        void resized() override {
            const auto &popupMetrics = theme.metrics.popup;
            auto bounds = getLocalBounds().reduced(static_cast<int>(popupMetrics.padding));

            auto layoutSection = [&](juce::Label &header, const std::initializer_list<int> indices) {
                if (header.isVisible()) {
                    header.setBounds(bounds.removeFromTop(static_cast<int>(popupMetrics.headerHeight)));
                    bounds.removeFromTop(static_cast<int>(popupMetrics.rowGap));
                }

                bool anyVisible = false;
                for (const auto index: indices) {
                    auto &button = buttons[static_cast<size_t>(index)];
                    if (!button->isVisible())
                        continue;

                    button->setBounds(bounds.removeFromTop(static_cast<int>(popupMetrics.rowHeight)));
                    bounds.removeFromTop(static_cast<int>(popupMetrics.rowGap));
                    anyVisible = true;
                }

                if (anyVisible)
                    bounds.removeFromTop(static_cast<int>(popupMetrics.sectionGap - popupMetrics.rowGap));
            };

            layoutSection(mainLabel, {0, 1, 2});
            if (sidechainAvailable)
                layoutSection(sidechainLabel, {3, 4, 5});
        }

        int getPreferredHeight() const {
            const auto &popupMetrics = theme.metrics.popup;
            if (!sidechainAvailable)
                return static_cast<int>(popupMetrics.padding * 2 + popupMetrics.rowHeight * 3 + popupMetrics.rowGap * 2);

            return static_cast<int>(popupMetrics.padding * 2
                                    + popupMetrics.headerHeight * 2
                                    + popupMetrics.rowHeight * 6
                                    + popupMetrics.rowGap * 6
                                    + popupMetrics.sectionGap);
        }

        int getPreferredWidth() const {
            return 132;
        }

    private:
        const Ui::Theme &theme;
        bool sidechainAvailable = false;
        juce::Label mainLabel;
        juce::Label sidechainLabel;
        std::array<std::unique_ptr<SignalSelectionRowButton>, 6> buttons;
        std::function<void(Analyzer::SignalSource, Analyzer::SignalMode)> onSelect;
        std::function<void()> onDismiss;
    };

    class SignalColourButton final : public juce::Button {
    public:
        SignalColourButton(const juce::Colour colourToUse, const bool selectedToUse)
            : juce::Button({}), colour(colourToUse), selected(selectedToUse) {
        }

        void paintButton(juce::Graphics &g, bool isMouseOverButton, bool) override {
            auto bounds = getLocalBounds().toFloat().reduced(4.0f);

            g.setColour(colour.withMultipliedAlpha(isEnabled() ? 1.0f : 0.28f));
            g.fillEllipse(bounds);

            if (isMouseOverButton) {
                g.setColour(juce::Colours::white.withAlpha(0.12f));
                g.fillEllipse(bounds.reduced(2.0f));
            }

            g.setColour(selected ? juce::Colours::white : juce::Colours::white.withAlpha(0.14f));
            g.drawEllipse(bounds, selected ? 2.0f : 1.0f);

            if (!isEnabled()) {
                g.setColour(juce::Colours::white.withAlpha(0.18f));
                g.drawLine(bounds.getX() + 5.0f, bounds.getBottom() - 5.0f,
                           bounds.getRight() - 5.0f, bounds.getY() + 5.0f, 1.5f);
            }
        }

    private:
        juce::Colour colour;
        bool selected = false;
    };

    class SignalColourCalloutContent final : public juce::Component {
    public:
        SignalColourCalloutContent(const Ui::Theme &themeToUse,
                                   std::function<void(int)> onSelectToUse,
                                   std::function<void()> onDismissToUse)
            : onSelect(std::move(onSelectToUse)),
              theme(themeToUse),
              onDismiss(std::move(onDismissToUse)) {
        }

        ~SignalColourCalloutContent() override {
            if (onDismiss)
                onDismiss();
        }

        void addColourButton(std::unique_ptr<SignalColourButton> button) {
            addAndMakeVisible(*button);
            colourButtons.push_back(std::move(button));
        }

        void resized() override {
            const auto &popupMetrics = theme.metrics.popup;
            auto bounds = getLocalBounds().reduced(static_cast<int>(popupMetrics.padding));
            const auto columns = popupMetrics.colourColumns;
            const auto itemSize = static_cast<int>(popupMetrics.swatchSize);
            const auto gap = static_cast<int>(popupMetrics.colourGap);

            auto x = bounds.getX();
            auto y = bounds.getY();
            for (size_t index = 0; index < colourButtons.size(); ++index) {
                colourButtons[index]->setBounds(x, y, itemSize, itemSize);
                x += itemSize + gap;
                if ((index + 1) % static_cast<size_t>(columns) == 0) {
                    x = bounds.getX();
                    y += itemSize + gap;
                }
            }
        }

        std::function<void(int)> onSelect;

        int getPreferredWidth() const {
            const auto &popupMetrics = theme.metrics.popup;
            return static_cast<int>(popupMetrics.padding * 2
                                    + popupMetrics.swatchSize * static_cast<float>(popupMetrics.colourColumns)
                                    + popupMetrics.colourGap * static_cast<float>(popupMetrics.colourColumns - 1));
        }

        int getPreferredHeight() const {
            const auto &popupMetrics = theme.metrics.popup;
            constexpr int rows = 2;
            return static_cast<int>(popupMetrics.padding * 2
                                    + popupMetrics.swatchSize * static_cast<float>(rows)
                                    + popupMetrics.colourGap * static_cast<float>(rows - 1));
        }

    private:
        const Ui::Theme &theme;
        std::vector<std::unique_ptr<SignalColourButton>> colourButtons;
        std::function<void()> onDismiss;
    };
}

SignalSlotComponent::SignalSlotComponent(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
}

void SignalSlotComponent::setSlot(const size_t slotIndexToUse,
                                  const Ui::SignalSlotState &settingsToUse,
                                  const std::vector<int> &usedColoursToUse,
                                  const std::vector<std::pair<Analyzer::SignalSource, Analyzer::SignalMode>> &usedSignalConfigsToUse) {
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
    const auto modeWidth = std::max({
        measureTextWidth(slotMetrics.titleFontHeight, "Mid"),
        measureTextWidth(slotMetrics.titleFontHeight, "Side"),
        measureTextWidth(slotMetrics.titleFontHeight, "Stereo")
    });
    const auto hintWidth = std::max({
        measureTextWidth(slotMetrics.hintFontHeight, "Main"),
        measureTextWidth(slotMetrics.hintFontHeight, "Sidechain")
    });
    const auto textWidth = std::ceil(std::max(modeWidth, hintWidth));
    const auto actionWidth = slotMetrics.actionSize * 3.0f + slotMetrics.actionGap * 2.0f;

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
    g.drawText(getModeLabel(settings.configuration.mode), labelBounds.removeFromTop(static_cast<int>(slotMetrics.titleHeight)).toNearestInt(),
               juce::Justification::centredLeft);
    labelBounds.removeFromTop(static_cast<int>(slotMetrics.textStackGap));

    g.setColour(theme.subtleText);
    g.setFont(slotMetrics.hintFontHeight);
    g.drawText(getSourceHint(settings.configuration.source), labelBounds.removeFromTop(static_cast<int>(slotMetrics.hintHeight)).toNearestInt(),
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
    contentBounds.removeFromRight(slotMetrics.actionSize * 3.0f + slotMetrics.actionGap * 2.0f + slotMetrics.sectionGap);
    const auto textHeight = slotMetrics.titleHeight + slotMetrics.textStackGap + slotMetrics.hintHeight;
    const auto y = contentBounds.getCentreY() - textHeight * 0.5f;
    return {contentBounds.getX(), y, contentBounds.getWidth(), textHeight};
}

juce::Rectangle<float> SignalSlotComponent::getDragHandleBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto contentBounds = getLocalBounds().toFloat().reduced(slotMetrics.cellPaddingX, slotMetrics.cellPaddingY);
    const auto actionsWidth = slotMetrics.actionSize * 3.0f + slotMetrics.actionGap * 2.0f;
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

juce::Rectangle<float> SignalSlotComponent::getRemoveBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto visibilityBounds = getVisibilityBounds();
    const auto x = visibilityBounds.getRight() + slotMetrics.actionGap;
    const auto y = visibilityBounds.getCentreY() - slotMetrics.actionSize * 0.5f;
    return {x, y, slotMetrics.actionSize, slotMetrics.actionSize};
}

SignalSlotHitArea SignalSlotComponent::getHitAreaAt(const juce::Point<float> &position) const {
    if (getSwatchBounds().contains(position))
        return SignalSlotHitArea::swatch;

    if (getDragHandleBounds().contains(position))
        return SignalSlotHitArea::dragHandle;

    if (getVisibilityBounds().contains(position))
        return SignalSlotHitArea::visibility;

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
    auto content = std::make_unique<SignalSelectionCalloutContent>(
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
    auto content = std::make_unique<SignalColourCalloutContent>(
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
        auto button = std::make_unique<SignalColourButton>(Ui::getSignalPresetColour(colourIndex),
                                                           colourIndex == settings.colourIndex);
        button->setEnabled(isColourAvailable(colourIndex));
        button->onClick = [contentPtr = content.get(), colourIndex] {
            if (!contentPtr->onSelect)
                return;

            contentPtr->onSelect(colourIndex);

            if (auto *callout = contentPtr->findParentComponentOfClass<juce::CallOutBox>())
                callout->dismiss();
        };
        content->addColourButton(std::move(button));
    }

    content->setSize(content->getPreferredWidth(), content->getPreferredHeight());

    auto anchorBounds = parentComponent->getLocalArea(this, getSwatchBounds().toNearestInt());
    anchorBounds = {anchorBounds.getCentreX(), anchorBounds.getY() - 2, 1, 1};
    launchCallout(std::move(content), OpenPopupMenu::colour, anchorBounds);
}

juce::String SignalSlotComponent::getModeLabel(const Analyzer::SignalMode mode) {
    switch (mode) {
        case Analyzer::SignalMode::mid:
            return "Mid";
        case Analyzer::SignalMode::side:
            return "Side";
        case Analyzer::SignalMode::stereo:
            return "Stereo";
    }

    return "Mid";
}

juce::String SignalSlotComponent::getSourceHint(const Analyzer::SignalSource source) {
    return source == Analyzer::SignalSource::main ? "Main" : "Sidechain";
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

    setMouseCursor(juce::MouseCursor::NormalCursor);
}
