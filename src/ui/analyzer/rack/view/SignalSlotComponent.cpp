#include "SignalSlotComponent.h"

#include <algorithm>
#include <cmath>

#include "ui/theme/UiRasterAssets.h"
#include "ui/analyzer/rack/popups/SignalColourPopupContent.h"
#include "ui/analyzer/rack/popups/SignalModeSelectionPopupContent.h"

namespace {
    juce::Rectangle<int> getCenteredSquareBounds(const juce::Rectangle<float> laneBounds, const float size) {
        return juce::Rectangle<float>(size, size).withCentre(laneBounds.getCentre()).toNearestInt();
    }
}

SignalSlotComponent::SignalSlotComponent(const Ui::Theme &themeToUse)
    : theme(themeToUse),
      popupLookAndFeel(themeToUse),
      sourceToggle(themeToUse),
      modeButton(themeToUse),
      swatchButton(themeToUse),
      dragHandle(themeToUse),
      visibilityButton(themeToUse, {}),
      freezeButton(themeToUse, {}),
      removeButton(themeToUse) {
    addAndMakeVisible(sourceToggle);
    addAndMakeVisible(modeButton);
    addAndMakeVisible(swatchButton);
    addAndMakeVisible(dragHandle);
    addAndMakeVisible(visibilityButton);
    addAndMakeVisible(freezeButton);
    addAndMakeVisible(removeButton);

    sourceToggle.onSourceSelected = [this](const Analyzer::SignalSource source) { handleSourceSelected(source); };
    modeButton.onPress = [this] { handleModePressed(); };
    modeButton.onClick = [this] { handleModeClicked(); };
    swatchButton.onPress = [this] {
        if (openPopupMenu != OpenPopupMenu::colour)
            return;

        dismissOpenMenu();
        suppressNextSwatchClick = true;
    };
    swatchButton.onClick = [this] { handleSwatchClicked(); };
    swatchButton.onOpacityChanged = [this](const float opacity) { handleOpacityChanged(opacity); };
    swatchButton.onOpacityReset = [this] { handleOpacityReset(); };
    dragHandle.onDragStarted = [this](const float parentX) { handleReorderDragStarted(parentX); };
    dragHandle.onDragged = [this](const float parentX) { handleReorderDragged(parentX); };
    dragHandle.onDragEnded = [this](const float parentX) { handleReorderDragEnded(parentX); };
    visibilityButton.onClick = [this] { handleVisibilityClicked(); };
    freezeButton.onClick = [this] { handleFreezeClicked(); };
    removeButton.onClick = [this] { handleRemoveClicked(); };

    visibilityButton.setScaleMultiplier(theme.metrics.slot.actionPadScaleMultiplier);
    visibilityButton.setOverlayIconScaleMultiplier(theme.metrics.slot.actionPadIconScaleMultiplier);
    visibilityButton.setOverlayIcon(PadButton::OverlayIcon::power);

    freezeButton.setAssetStyle(PadButton::AssetStyle::freeze);
    freezeButton.setScaleMultiplier(theme.metrics.slot.actionPadScaleMultiplier);
    freezeButton.setOverlayIconScaleMultiplier(theme.metrics.slot.actionPadIconScaleMultiplier);
    freezeButton.setOverlayIcon(PadButton::OverlayIcon::snowflake);
    freezeButton.setActiveMarkingColour(theme.hardwareMarkingCoolDark);
}

SignalSlotComponent::~SignalSlotComponent() {
    if (activeCallout != nullptr) {
        activeCallout->setLookAndFeel(nullptr);
        activeCallout->dismiss();
        activeCallout = nullptr;
    }
}

void SignalSlotComponent::setListener(Listener *listenerToUse) {
    listener = listenerToUse;
}

void SignalSlotComponent::setSlot(const size_t slotIndexToUse,
                                  const Ui::SignalSlotState &settingsToUse,
                                  const std::vector<int> &usedColoursToUse,
                                  const std::vector<Ui::SignalSlotKey> &usedSignalConfigsToUse) {
    slotIndex = slotIndexToUse;
    settings = settingsToUse;
    usedColours = usedColoursToUse;
    usedSignalConfigs = usedSignalConfigsToUse;
    refreshChildState();
}

void SignalSlotComponent::setSidechainAvailable(const bool isAvailable) {
    isSidechainRouted = isAvailable;
    refreshChildState();
}

void SignalSlotComponent::setDragged(const bool isDraggedValue) {
    isDragged = isDraggedValue;
    dragHandle.setDragged(isDraggedValue);
    repaint();
}

bool SignalSlotComponent::getDragged() const {
    return isDragged;
}

void SignalSlotComponent::paint(juce::Graphics &g) {
    const auto bounds = getModuleBounds();
    const auto radius = theme.metrics.slot.cellCornerRadius;
    const auto outerBounds = bounds.reduced(0.5f);
    juce::Path modulePath;
    modulePath.addRoundedRectangle(bounds, radius);

    if (isDragged) {
        g.setColour(juce::Colours::black.withAlpha(theme.metrics.slot.draggedShadowAlpha));
        g.fillRoundedRectangle(bounds.translated(0.0f, theme.metrics.slot.shadowOffsetY), radius);
    }

    if (cachedBackground.isValid()) {
        juce::Graphics::ScopedSaveState saveState(g);
        g.reduceClipRegion(modulePath);
        g.drawImage(cachedBackground, bounds);
    }

    g.setColour(theme.controlBorder.withMultipliedAlpha(theme.metrics.slot.borderAlphaScale));
    g.drawRoundedRectangle(outerBounds, radius, 1.0f);
}

void SignalSlotComponent::resized() {
    rebuildCachedBackground();

    const auto &slotMetrics = theme.metrics.slot;
    const auto actionControlSize = slotMetrics.actionSize;
    const auto swatchControlSize = slotMetrics.swatchSize;
    const auto moduleBounds = getModuleBounds();
    auto modeRowBounds = getModeRowBounds();
    auto actionRowBounds = getActionRowBounds();

    sourceToggle.setBounds(getSourceToggleBounds().toNearestInt());

    const auto removeButtonBounds = juce::Rectangle<float>(
        moduleBounds.getRight() - slotMetrics.topRowEdgeInset - actionControlSize,
        modeRowBounds.getY() + (modeRowBounds.getHeight() - actionControlSize) * 0.5f,
        actionControlSize,
        actionControlSize);
    removeButton.setBounds(removeButtonBounds.toNearestInt());

    const auto modeButtonRight = removeButtonBounds.getX() - slotMetrics.modeActionGap;
    const auto modeButtonBounds = juce::Rectangle<float>(
        modeButtonRight - slotMetrics.modeDisplayWidth,
        modeRowBounds.getY() + (modeRowBounds.getHeight() - slotMetrics.modeDisplayHeight) * 0.5f,
        slotMetrics.modeDisplayWidth,
        slotMetrics.modeDisplayHeight);
    modeButton.setBounds(modeButtonBounds.toNearestInt());

    auto actionClusterBounds = juce::Rectangle<float>(
        modeButtonBounds.getRight() - slotMetrics.modeDisplayWidth,
        actionRowBounds.getY(),
        slotMetrics.modeDisplayWidth,
        actionRowBounds.getHeight());

    swatchButton.setBounds(getCenteredSquareBounds(actionClusterBounds.removeFromLeft(swatchControlSize), swatchControlSize));
    actionClusterBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionGap)));
    dragHandle.setBounds(getCenteredSquareBounds(actionClusterBounds.removeFromLeft(actionControlSize), actionControlSize));
    actionClusterBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionGap)));
    visibilityButton.setBounds(getCenteredSquareBounds(actionClusterBounds.removeFromLeft(actionControlSize), actionControlSize));
    actionClusterBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionGap)));
    freezeButton.setBounds(getCenteredSquareBounds(actionClusterBounds.removeFromLeft(actionControlSize), actionControlSize));
}

juce::Rectangle<float> SignalSlotComponent::getModuleBounds() const {
    auto bounds = getLocalBounds().toFloat();
    bounds.removeFromBottom(theme.metrics.rack.bottomInset);
    return bounds;
}

void SignalSlotComponent::rebuildCachedBackground() {
    const auto moduleBounds = getModuleBounds();
    if (moduleBounds.isEmpty()) {
        cachedBackground = {};
        return;
    }

    const auto targetBounds = moduleBounds.getSmallestIntegerContainer();
    cachedBackground = juce::Image(juce::Image::ARGB, targetBounds.getWidth(), targetBounds.getHeight(), true);
    juce::Graphics graphics(cachedBackground);
    const auto &backgroundImage = Ui::getAnalyzerRasterAsset(Ui::AnalyzerRasterAssetId::background2Version);
    graphics.drawImage(backgroundImage,
                       0,
                       0,
                       targetBounds.getWidth(),
                       targetBounds.getHeight(),
                       0,
                       0,
                       backgroundImage.getWidth(),
                       backgroundImage.getHeight());

    const auto &screwImage = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::screw);
    const auto screwPadding = juce::roundToInt(theme.metrics.slot.screwPadding);
    const auto topLeftScrewBounds = Ui::getScaledAssetBoundsWithin(screwImage,
                                                                   theme.metrics.assets.rasterScale,
                                                                   {screwPadding,
                                                                    screwPadding,
                                                                    targetBounds.getWidth(),
                                                                    targetBounds.getHeight()},
                                                                   theme.metrics.slot.screwScale)
                                        .withPosition(screwPadding, screwPadding);
    const auto topRightScrewBounds = topLeftScrewBounds.withX(targetBounds.getWidth()
                                                              - screwPadding
                                                              - topLeftScrewBounds.getWidth());

    Ui::drawAssetWithin(graphics, screwImage, topLeftScrewBounds);
    Ui::drawAssetWithin(graphics, screwImage, topRightScrewBounds);
}

void SignalSlotComponent::refreshChildState() {
    sourceToggle.setState(settings.configuration.source, isSidechainRouted);
    modeButton.setLabel(Ui::getSignalModeLabel(settings.configuration.mode));
    swatchButton.setState(settings.colourIndex, settings.opacity);
    dragHandle.setDragged(isDragged);
    visibilityButton.setActive(settings.visible);
    freezeButton.setActive(settings.frozen);

    SignalSlotActionButton::Style removeStyle;
    removeStyle.content = SignalSlotActionButton::Content::cancel;
    removeStyle.fill = juce::Colours::transparentBlack;
    removeStyle.hoverFill = juce::Colours::transparentBlack;
    removeStyle.foreground = theme.hardwareMarkingLight;
    removeStyle.drawsBackground = false;
    removeButton.setStyle(removeStyle);

    repaint();
}

juce::Rectangle<float> SignalSlotComponent::getContentBounds() const {
    auto bounds = getModuleBounds().reduced(theme.metrics.slot.cellPaddingX, theme.metrics.slot.cellPaddingY);
    bounds.removeFromTop(theme.metrics.slot.contentOffsetY);
    return bounds;
}

juce::Rectangle<float> SignalSlotComponent::getSourceToggleBounds() const {
    auto contentBounds = getContentBounds();
    contentBounds.setWidth(theme.metrics.slot.sourceToggleWidth);
    return contentBounds;
}

juce::Rectangle<float> SignalSlotComponent::getControlColumnBounds() const {
    auto contentBounds = getContentBounds();
    contentBounds.removeFromLeft(theme.metrics.slot.sourceToggleWidth);
    contentBounds.removeFromLeft(theme.metrics.slot.sectionGap);
    return contentBounds;
}

juce::Rectangle<float> SignalSlotComponent::getControlStackBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    const auto controlColumnBounds = getControlColumnBounds();
    const auto stackHeight = slotMetrics.modeDisplayHeight + slotMetrics.rowGap + getActionRowHeight();
    return juce::Rectangle<float>(controlColumnBounds.getWidth(), stackHeight)
        .withCentre(controlColumnBounds.getCentre());
}

float SignalSlotComponent::getActionRowHeight() const {
    return juce::jmax(theme.metrics.slot.actionSize, theme.metrics.slot.swatchSize);
}

juce::Rectangle<float> SignalSlotComponent::getModeRowBounds() const {
    auto modeRowBounds = getControlStackBounds();
    modeRowBounds.setHeight(theme.metrics.slot.modeDisplayHeight);
    return modeRowBounds;
}

juce::Rectangle<float> SignalSlotComponent::getActionRowBounds() const {
    const auto &slotMetrics = theme.metrics.slot;
    auto rowStackBounds = getControlStackBounds();
    rowStackBounds.removeFromTop(slotMetrics.modeDisplayHeight + slotMetrics.rowGap);
    rowStackBounds.setHeight(getActionRowHeight());
    return rowStackBounds;
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

void SignalSlotComponent::dismissOpenMenu() {
    if (openPopupMenu == OpenPopupMenu::none)
        return;

    openPopupMenu = OpenPopupMenu::none;
    if (activeCallout != nullptr) {
        activeCallout->setLookAndFeel(nullptr);
        activeCallout->dismiss();
    }

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
    callout.setLookAndFeel(&popupLookAndFeel);
    callout.lookAndFeelChanged();
    callout.setDismissalMouseClicksAreAlwaysConsumed(false);
    activeCallout = &callout;
}

void SignalSlotComponent::showModeMenu() {
    auto *parentComponent = getTopLevelComponent();
    if (parentComponent == nullptr)
        parentComponent = getParentComponent();

    if (parentComponent == nullptr)
        return;

    dismissOpenMenu();

    const auto safeThis = juce::Component::SafePointer<SignalSlotComponent>(this);
    auto content = std::make_unique<SignalModeSelectionPopupContent>(
        theme,
        settings.configuration.source,
        settings.configuration.mode,
        [safeThis](const Analyzer::SignalMode mode) {
            if (safeThis == nullptr)
                return;

            safeThis->settings.configuration.mode = mode;
            safeThis->refreshChildState();

            if (safeThis->listener != nullptr)
                safeThis->listener->signalSlotModeSelected(safeThis->slotIndex, mode);
        },
        [safeThis] {
            if (safeThis == nullptr)
                return;

            safeThis->openPopupMenu = OpenPopupMenu::none;
            safeThis->activeCallout = nullptr;
        });

    content->setAvailability([this](const Analyzer::SignalMode mode) {
        return isSignalAvailable(settings.configuration.source, mode);
    });
    content->setSize(content->getPreferredWidth(), content->getPreferredHeight());

    auto anchorBounds = parentComponent->getLocalArea(this, modeButton.getBounds());
    anchorBounds = {anchorBounds.getCentreX(), anchorBounds.getY(), 1, 1};
    launchCallout(std::move(content), OpenPopupMenu::mode, anchorBounds);
}

void SignalSlotComponent::showColourMenu() {
    auto *parentComponent = getTopLevelComponent();
    if (parentComponent == nullptr)
        parentComponent = getParentComponent();

    if (parentComponent == nullptr)
        return;

    dismissOpenMenu();

    const auto safeThis = juce::Component::SafePointer<SignalSlotComponent>(this);
    auto content = std::make_unique<SignalColourPopupContent>(
        theme,
        [safeThis](const int selectedColourIndex) {
            if (safeThis == nullptr)
                return;

            safeThis->settings.colourIndex = selectedColourIndex;
            safeThis->refreshChildState();

            if (safeThis->listener != nullptr)
                safeThis->listener->signalSlotColourSelected(safeThis->slotIndex, selectedColourIndex);
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

    auto anchorBounds = parentComponent->getLocalArea(this, swatchButton.getBounds());
    anchorBounds = {anchorBounds.getCentreX(), anchorBounds.getY() - 2, 1, 1};
    launchCallout(std::move(content), OpenPopupMenu::colour, anchorBounds);
}

void SignalSlotComponent::handleSourceSelected(const Analyzer::SignalSource source) {
    if (settings.configuration.source == source)
        return;

    settings.configuration.source = source;
    refreshChildState();

    if (listener != nullptr)
        listener->signalSlotSourceSelected(slotIndex, source);
}

void SignalSlotComponent::handleModePressed() {
    if (openPopupMenu != OpenPopupMenu::mode)
        return;

    dismissOpenMenu();
    suppressNextModeButtonClick = true;
}

void SignalSlotComponent::handleModeClicked() {
    if (suppressNextModeButtonClick) {
        suppressNextModeButtonClick = false;
        return;
    }

    showModeMenu();
}

void SignalSlotComponent::handleVisibilityClicked() {
    settings.visible = !settings.visible;
    refreshChildState();

    if (listener != nullptr)
        listener->signalSlotVisibilityChanged(slotIndex, settings.visible);
}

void SignalSlotComponent::handleFreezeClicked() {
    settings.frozen = !settings.frozen;
    refreshChildState();

    if (listener != nullptr)
        listener->signalSlotFrozenChanged(slotIndex, settings.frozen);
}

void SignalSlotComponent::handleRemoveClicked() {
    if (listener != nullptr)
        listener->signalSlotRemoveRequested(slotIndex);
}

void SignalSlotComponent::handleSwatchClicked() {
    if (suppressNextSwatchClick) {
        suppressNextSwatchClick = false;
        return;
    }

    showColourMenu();
}

void SignalSlotComponent::handleOpacityChanged(const float opacity) {
    settings.opacity = opacity;
    refreshChildState();

    if (listener != nullptr)
        listener->signalSlotOpacityChanged(slotIndex, opacity);
}

void SignalSlotComponent::handleOpacityReset() {
    settings.opacity = Ui::defaultSignalOpacity;
    refreshChildState();

    if (listener != nullptr)
        listener->signalSlotOpacityChanged(slotIndex, settings.opacity);
}

void SignalSlotComponent::handleReorderDragStarted(const float parentX) {
    if (listener != nullptr)
        listener->signalSlotReorderDragStarted(slotIndex, parentX);
}

void SignalSlotComponent::handleReorderDragged(const float parentX) {
    if (listener != nullptr)
        listener->signalSlotReorderDragged(parentX);
}

void SignalSlotComponent::handleReorderDragEnded(const float parentX) {
    if (listener != nullptr)
        listener->signalSlotReorderDragEnded(parentX);
}
