#include "SignalSlotComponent.h"

#include <algorithm>
#include <cmath>

#include "../../UiRasterAssets.h"
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
    : theme(themeToUse),
      popupLookAndFeel(themeToUse),
      sourceToggle(themeToUse),
      modeButton(themeToUse),
      swatchButton(themeToUse),
      dragHandle(themeToUse),
      visibilityButton(themeToUse),
      freezeButton(themeToUse),
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
    swatchButton.onClick = [this] { handleSwatchClicked(); };
    swatchButton.onOpacityChanged = [this](const float opacity) { handleOpacityChanged(opacity); };
    swatchButton.onOpacityReset = [this] { handleOpacityReset(); };
    dragHandle.onDragStarted = [this](const float parentX) { handleReorderDragStarted(parentX); };
    dragHandle.onDragged = [this](const float parentX) { handleReorderDragged(parentX); };
    dragHandle.onDragEnded = [this](const float parentX) { handleReorderDragEnded(parentX); };
    visibilityButton.onClick = [this] { handleVisibilityClicked(); };
    freezeButton.onClick = [this] { handleFreezeClicked(); };
    removeButton.onClick = [this] { handleRemoveClicked(); };
}

SignalSlotComponent::~SignalSlotComponent() {
    if (activeCallout != nullptr) {
        activeCallout->setLookAndFeel(nullptr);
        activeCallout->dismiss();
        activeCallout = nullptr;
    }
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
    const auto bounds = getModuleBounds();
    const auto radius = theme.metrics.slot.cellCornerRadius;
    const auto outerBounds = bounds.reduced(0.5f);
    juce::Path modulePath;
    modulePath.addRoundedRectangle(bounds, radius);

    if (isDragged) {
        g.setColour(juce::Colours::black.withAlpha(0.22f));
        g.fillRoundedRectangle(bounds.translated(0.0f, theme.metrics.slot.shadowOffsetY), radius);
    }

    if (cachedBackground.isValid()) {
        juce::Graphics::ScopedSaveState saveState(g);
        g.reduceClipRegion(modulePath);
        g.drawImage(cachedBackground, bounds);
    }

    g.setColour(theme.controlBorder.withMultipliedAlpha(1.35f));
    g.drawRoundedRectangle(outerBounds, radius, 1.0f);
}

void SignalSlotComponent::resized() {
    rebuildCachedBackground();

    const auto &slotMetrics = theme.metrics.slot;
    auto sourceToggleBounds = getSourceToggleBounds();
    auto topRowBounds = getTopRowBounds();
    auto bottomRowBounds = getBottomRowBounds();

    sourceToggle.setBounds(sourceToggleBounds.toNearestInt());
    topRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.sourceToggleWidth)));
    topRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.sectionGap)));
    modeButton.setBounds(topRowBounds.toNearestInt());

    swatchButton.setBounds(bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionSize))).toNearestInt());
    bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionGap)));
    dragHandle.setBounds(bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionSize))).toNearestInt());
    bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionGap)));
    visibilityButton.setBounds(bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionSize))).toNearestInt());
    bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionGap)));
    freezeButton.setBounds(bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionSize))).toNearestInt());
    bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionGap)));
    removeButton.setBounds(bottomRowBounds.removeFromLeft(static_cast<int>(std::round(slotMetrics.actionSize))).toNearestInt());
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
    const auto &backgroundImage = Ui::getRasterAsset(Ui::RasterAssetId::background2Version);
    graphics.drawImage(backgroundImage,
                       0,
                       0,
                       targetBounds.getWidth(),
                       targetBounds.getHeight(),
                       0,
                       0,
                       backgroundImage.getWidth(),
                       backgroundImage.getHeight());
}

void SignalSlotComponent::refreshChildState() {
    sourceToggle.setState(settings.configuration.source, isSidechainRouted);
    modeButton.setLabel(Ui::getSignalModeLabel(settings.configuration.mode));
    swatchButton.setState(settings.colourIndex, settings.opacity);
    dragHandle.setDragged(isDragged);

    SignalSlotActionButton::Style visibilityStyle;
    const auto visibilityIconStyle = Ui::getIconActionButtonStyle(theme, settings.visible, false);
    visibilityStyle.content = SignalSlotActionButton::Content::power;
    visibilityStyle.fill = visibilityIconStyle.fill;
    visibilityStyle.hoverFill = Ui::getIconActionButtonStyle(theme, settings.visible, true).fill;
    visibilityStyle.foreground = visibilityIconStyle.icon;
    visibilityButton.setStyle(visibilityStyle);

    SignalSlotActionButton::Style freezeStyle;
    const auto freezeIconStyle = Ui::getIconActionButtonStyle(theme, settings.frozen, false);
    freezeStyle.content = SignalSlotActionButton::Content::snowflake;
    freezeStyle.fill = freezeIconStyle.fill;
    freezeStyle.hoverFill = Ui::getIconActionButtonStyle(theme, settings.frozen, true).fill;
    freezeStyle.foreground = freezeIconStyle.icon;
    freezeButton.setStyle(freezeStyle);

    SignalSlotActionButton::Style removeStyle;
    removeStyle.content = SignalSlotActionButton::Content::cancel;
    removeStyle.fill = theme.controlSurface;
    removeStyle.hoverFill = theme.controlSurfaceHover;
    removeStyle.foreground = theme.subtleText;
    removeButton.setStyle(removeStyle);

    repaint();
}

juce::Rectangle<float> SignalSlotComponent::getContentBounds() const {
    return getModuleBounds().reduced(theme.metrics.slot.cellPaddingX, theme.metrics.slot.cellPaddingY);
}

juce::Rectangle<float> SignalSlotComponent::getSourceToggleBounds() const {
    auto contentBounds = getContentBounds();
    contentBounds.setWidth(theme.metrics.slot.sourceToggleWidth);
    contentBounds.setHeight(juce::jmin(contentBounds.getHeight(), theme.metrics.slot.sourceToggleHeight));
    return contentBounds;
}

juce::Rectangle<float> SignalSlotComponent::getTopRowBounds() const {
    auto contentBounds = getContentBounds();
    contentBounds.setHeight(theme.metrics.slot.topRowHeight);
    return contentBounds;
}

juce::Rectangle<float> SignalSlotComponent::getBottomRowBounds() const {
    auto contentBounds = getContentBounds();
    const auto y = contentBounds.getBottom() - theme.metrics.slot.actionSize;
    return {contentBounds.getX(), y, contentBounds.getWidth(), theme.metrics.slot.actionSize};
}

float SignalSlotComponent::getActionClusterWidth() const {
    return theme.metrics.slot.actionSize * 5.0f + theme.metrics.slot.actionGap * 4.0f;
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

void SignalSlotComponent::showSignalMenu() {
    auto *parentComponent = getTopLevelComponent();
    if (parentComponent == nullptr)
        parentComponent = getParentComponent();

    if (parentComponent == nullptr)
        return;

    dismissOpenMenu();

    const auto safeThis = juce::Component::SafePointer<SignalSlotComponent>(this);
    auto content = std::make_unique<SignalSelectionPopupContent>(
        theme,
        settings.configuration.source,
        settings.configuration.mode,
        [safeThis](const Analyzer::SignalSource source, const Analyzer::SignalMode mode) {
            if (safeThis == nullptr)
                return;

            safeThis->settings.configuration.source = source;
            safeThis->settings.configuration.mode = mode;
            safeThis->refreshChildState();

            if (safeThis->onSignalSelected)
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

            if (safeThis->onColourSelected)
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

    auto anchorBounds = parentComponent->getLocalArea(this, swatchButton.getBounds());
    anchorBounds = {anchorBounds.getCentreX(), anchorBounds.getY() - 2, 1, 1};
    launchCallout(std::move(content), OpenPopupMenu::colour, anchorBounds);
}

void SignalSlotComponent::handleSourceSelected(const Analyzer::SignalSource source) {
    if (settings.configuration.source == source)
        return;

    settings.configuration.source = source;
    refreshChildState();

    if (onSignalSelected)
        onSignalSelected(slotIndex, settings.configuration.source, settings.configuration.mode);
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

    showSignalMenu();
}

void SignalSlotComponent::handleVisibilityClicked() {
    settings.visible = !settings.visible;
    refreshChildState();

    if (onVisibilityChanged)
        onVisibilityChanged(slotIndex, settings.visible);
}

void SignalSlotComponent::handleFreezeClicked() {
    settings.frozen = !settings.frozen;
    refreshChildState();

    if (onFrozenChanged)
        onFrozenChanged(slotIndex, settings.frozen);
}

void SignalSlotComponent::handleRemoveClicked() {
    if (onRemoveClicked)
        onRemoveClicked(slotIndex);
}

void SignalSlotComponent::handleSwatchClicked() {
    showColourMenu();
}

void SignalSlotComponent::handleOpacityChanged(const float opacity) {
    settings.opacity = opacity;
    refreshChildState();

    if (onOpacityChanged)
        onOpacityChanged(slotIndex, opacity);
}

void SignalSlotComponent::handleOpacityReset() {
    settings.opacity = Ui::defaultSignalOpacity;
    refreshChildState();

    if (onOpacityChanged)
        onOpacityChanged(slotIndex, settings.opacity);
}

void SignalSlotComponent::handleReorderDragStarted(const float parentX) {
    if (onReorderDragStarted)
        onReorderDragStarted(slotIndex, parentX);
}

void SignalSlotComponent::handleReorderDragged(const float parentX) {
    if (onReorderDragged)
        onReorderDragged(slotIndex, parentX);
}

void SignalSlotComponent::handleReorderDragEnded(const float parentX) {
    if (onReorderDragEnded)
        onReorderDragEnded(slotIndex, parentX);
}
