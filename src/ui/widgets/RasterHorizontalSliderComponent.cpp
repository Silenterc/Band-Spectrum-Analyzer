#include "RasterHorizontalSliderComponent.h"

#include "ui/theme/UiRasterAssets.h"

namespace {
    Ui::RasterFilmstripSpec getHorizontalSliderFilmstripSpec(const Ui::HorizontalSliderMetrics& metrics) {
        return {
            .frameCount = metrics.filmstripFrameCount,
            .frameWidth = metrics.filmstripFrameWidth,
            .frameHeight = metrics.filmstripFrameHeight,
            .orientation = Ui::FilmstripOrientation::vertical
        };
    }
}

RasterHorizontalSliderComponent::InlineValueEditor::InlineValueEditor(RasterHorizontalSliderComponent& ownerToUse)
    : owner(ownerToUse) {
}

void RasterHorizontalSliderComponent::InlineValueEditor::focusLost(const FocusChangeType cause) {
    juce::TextEditor::focusLost(cause);
    owner.commitValueEdit();
}

RasterHorizontalSliderComponent::OutsideClickListener::OutsideClickListener(RasterHorizontalSliderComponent& ownerToUse)
    : owner(ownerToUse) {
}

void RasterHorizontalSliderComponent::OutsideClickListener::mouseDown(const juce::MouseEvent& event) {
    if (owner.valueEditor.isVisible() && !owner.isPointInsideValueEditor(event))
        owner.commitValueEdit();
}

RasterHorizontalSliderComponent::RasterHorizontalSliderComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      valueEditor(*this),
      outsideClickListener(*this) {
    setWantsKeyboardFocus(true);
    setMouseCursor(juce::MouseCursor::NormalCursor);
    addChildComponent(valueEditor);
    configureEditor();
    updateLayout();
}

RasterHorizontalSliderComponent::~RasterHorizontalSliderComponent() {
    detachOutsideClickListener();
}

void RasterHorizontalSliderComponent::setConfig(Config newConfig) {
    config = std::move(newConfig);
    allowedMinimum = config.minimum;
    allowedMaximum = config.maximum;
    value = snapAndClamp(value);
    updateValueEditorText();
    repaint();
}

void RasterHorizontalSliderComponent::setValue(const float newValue,
                                               const juce::NotificationType notificationType) {
    setValueInternal(newValue, notificationType);
}

void RasterHorizontalSliderComponent::setAllowedRange(const float minimum, const float maximum) {
    allowedMinimum = minimum;
    allowedMaximum = maximum;
    setValueInternal(value, juce::dontSendNotification);
}

float RasterHorizontalSliderComponent::getValue() const {
    return value;
}

juce::Rectangle<int> RasterHorizontalSliderComponent::getPreferredBounds() const {
    return {0, 0, theme.metrics.horizontalSlider.width, theme.metrics.horizontalSlider.height};
}

void RasterHorizontalSliderComponent::paint(juce::Graphics& g) {
    const auto& metrics = theme.metrics.horizontalSlider;
    g.setColour(theme.hardwareMarkingLight);
    g.setFont(juce::FontOptions(metrics.labelFontHeight).withStyle("Bold"));
    g.drawText(config.label, labelBounds, juce::Justification::centred, false);

    const auto& sliderImage = Ui::getControlRasterAsset(Ui::ControlRasterAssetId::horizontalSliderFilmstrip);
    Ui::RasterFilmstrip::drawFrame(g,
                                   sliderImage,
                                   getHorizontalSliderFilmstripSpec(metrics),
                                   getFrameIndex(),
                                   sliderBounds);

    const auto& valueImage = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::textBox);
    if (valueImage.isValid())
        Ui::drawAssetWithin(g, valueImage, valueBounds);

    if (!valueEditor.isVisible()) {
        g.setColour(theme.hardwareMarkingLight);
        g.setFont(juce::FontOptions(metrics.valueFontHeight).withStyle("Bold"));
        g.drawText(formatValue(value), valueBounds, juce::Justification::centred, false);
    }
}

void RasterHorizontalSliderComponent::resized() {
    updateLayout();
}

void RasterHorizontalSliderComponent::mouseDown(const juce::MouseEvent& event) {
    if (valueBounds.contains(event.getPosition())) {
        beginValueEdit();
        return;
    }

    if (sliderBounds.contains(event.getPosition()))
        beginDrag(event);
}

void RasterHorizontalSliderComponent::mouseMove(const juce::MouseEvent& event) {
    updateMouseCursor(event.getPosition());
}

void RasterHorizontalSliderComponent::mouseExit(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void RasterHorizontalSliderComponent::mouseDrag(const juce::MouseEvent& event) {
    if (dragging)
        updateDrag(event);
}

void RasterHorizontalSliderComponent::mouseUp(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);
    if (dragging)
        endDrag();
}

bool RasterHorizontalSliderComponent::keyPressed(const juce::KeyPress& key) {
    const auto increment = key.getModifiers().isShiftDown()
                               ? config.step * theme.metrics.horizontalSlider.keyboardStepMultiplier
                               : config.step;
    if (increment <= 0.0f)
        return juce::Component::keyPressed(key);

    if (key == juce::KeyPress::upKey || key == juce::KeyPress::rightKey) {
        setValueInternal(value + increment, juce::sendNotification);
        return true;
    }

    if (key == juce::KeyPress::downKey || key == juce::KeyPress::leftKey) {
        setValueInternal(value - increment, juce::sendNotification);
        return true;
    }

    return juce::Component::keyPressed(key);
}

void RasterHorizontalSliderComponent::configureEditor() {
    const auto& metrics = theme.metrics.horizontalSlider;
    valueEditor.setSelectAllWhenFocused(true);
    valueEditor.setJustification(juce::Justification::centred);
    valueEditor.setMultiLine(false);
    valueEditor.setReturnKeyStartsNewLine(false);
    valueEditor.setScrollToShowCursor(true);
    valueEditor.setFont(juce::FontOptions(metrics.valueFontHeight, juce::Font::bold));
    valueEditor.applyFontToAllText(valueEditor.getFont(), true);
    valueEditor.setIndents(metrics.valueEditorTextIndentX,
                           metrics.valueEditorTextIndentTop);
    valueEditor.setColour(juce::TextEditor::backgroundColourId, theme.controlSurface);
    valueEditor.setColour(juce::TextEditor::outlineColourId,
                           theme.sectionDividerHighlight.withMultipliedAlpha(theme.metrics.presetPopup.editorOutlineAlpha));
    valueEditor.setColour(juce::TextEditor::focusedOutlineColourId,
                           theme.hardwareMarkingLight.withMultipliedAlpha(theme.metrics.presetPopup.editorFocusOutlineAlpha));
    valueEditor.setColour(juce::TextEditor::textColourId, theme.hardwareMarkingLight);
    valueEditor.setColour(juce::TextEditor::highlightColourId, theme.textSelectionFill);
    valueEditor.setColour(juce::TextEditor::highlightedTextColourId, theme.textSelectionText);
    valueEditor.onReturnKey = [this] { commitValueEdit(); };
    valueEditor.onEscapeKey = [this] { cancelValueEdit(); };
}

void RasterHorizontalSliderComponent::beginValueEdit() {
    if (valueEditor.isVisible())
        return;

    updateValueEditorText();
    valueEditor.setVisible(true);
    valueEditor.toFront(false);
    valueEditor.grabKeyboardFocus();
    valueEditor.selectAll();
    attachOutsideClickListener();
    repaint(valueBounds);
}

void RasterHorizontalSliderComponent::commitValueEdit() {
    if (!valueEditor.isVisible() || cancellingEdit)
        return;

    const auto parsedValue = parseValue(valueEditor.getText());
    detachOutsideClickListener();
    valueEditor.setVisible(false);
    if (parsedValue.has_value())
        setValueInternal(*parsedValue, juce::sendNotification);
    else
        updateValueEditorText();

    repaint(valueBounds);
}

void RasterHorizontalSliderComponent::cancelValueEdit() {
    if (!valueEditor.isVisible())
        return;

    cancellingEdit = true;
    detachOutsideClickListener();
    valueEditor.setVisible(false);
    updateValueEditorText();
    cancellingEdit = false;
    grabKeyboardFocus();
    repaint(valueBounds);
}

void RasterHorizontalSliderComponent::attachOutsideClickListener() {
    if (outsideClickListenerAttached)
        return;

    juce::Desktop::getInstance().addGlobalMouseListener(&outsideClickListener);
    outsideClickListenerAttached = true;
}

void RasterHorizontalSliderComponent::detachOutsideClickListener() {
    if (!outsideClickListenerAttached)
        return;

    juce::Desktop::getInstance().removeGlobalMouseListener(&outsideClickListener);
    outsideClickListenerAttached = false;
}

bool RasterHorizontalSliderComponent::isPointInsideValueEditor(const juce::MouseEvent& event) const {
    if (event.eventComponent == &valueEditor || valueEditor.isParentOf(event.eventComponent))
        return true;

    auto screenPoint = event.getScreenPosition();
    if (event.eventComponent != nullptr)
        screenPoint = event.eventComponent->localPointToGlobal(event.getPosition());

    return valueEditor.getScreenBounds().contains(screenPoint);
}

void RasterHorizontalSliderComponent::beginDrag(const juce::MouseEvent& event) {
    dragging = true;
    dragStartNormalised = normaliseValue(value);
    dragStartX = event.position.x;
    grabKeyboardFocus();
    if (onGestureStart != nullptr)
        onGestureStart();
}

void RasterHorizontalSliderComponent::updateDrag(const juce::MouseEvent& event) {
    const auto deltaNormalised = (event.position.x - dragStartX)
                                 / juce::jmax(1.0f, theme.metrics.horizontalSlider.dragPixelsForFullRange);
    setValueInternal(denormaliseValue(dragStartNormalised + deltaNormalised),
                     juce::sendNotification);
}

void RasterHorizontalSliderComponent::endDrag() {
    dragging = false;
    if (onGestureEnd != nullptr)
        onGestureEnd();
}

void RasterHorizontalSliderComponent::updateMouseCursor(const juce::Point<int> position) {
    setMouseCursor(isInteractivePosition(position) ? juce::MouseCursor::PointingHandCursor
                                                   : juce::MouseCursor::NormalCursor);
}

bool RasterHorizontalSliderComponent::isInteractivePosition(const juce::Point<int> position) const {
    return valueBounds.contains(position) || sliderBounds.contains(position);
}

void RasterHorizontalSliderComponent::updateLayout() {
    const auto& metrics = theme.metrics.horizontalSlider;
    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        bounds = getPreferredBounds();

    labelBounds = bounds.removeFromTop(metrics.labelHeight);
    bounds.removeFromTop(metrics.labelToSliderGap);

    const auto sliderSectionBounds = bounds.removeFromTop(metrics.sliderHeight);
    sliderBounds = juce::Rectangle<int>(metrics.sliderWidth, metrics.sliderHeight)
                       .withCentre({getLocalBounds().getCentreX(), sliderSectionBounds.getCentreY()});

    bounds.removeFromTop(metrics.sliderToValueGap);
    valueBounds = juce::Rectangle<int>(metrics.valueWidth, metrics.valueHeight)
                      .withCentre({getLocalBounds().getCentreX(), bounds.getY() + metrics.valueHeight / 2});
    valueEditor.setBounds(valueBounds);
}

void RasterHorizontalSliderComponent::updateValueEditorText() {
    valueEditor.setText(formatValue(value), false);
}

void RasterHorizontalSliderComponent::setValueInternal(const float newValue,
                                                       const juce::NotificationType notificationType) {
    const auto nextValue = snapAndClamp(newValue);
    if (juce::approximatelyEqual(value, nextValue))
        return;

    value = nextValue;
    updateValueEditorText();
    repaint();
    if (notificationType != juce::dontSendNotification && onValueChanged != nullptr)
        onValueChanged(value);
}

float RasterHorizontalSliderComponent::snapAndClamp(const float plainValue) const {
    const auto snapped = Ui::RasterSliderValueMapping::snapValue(plainValue, config.minimum, config.maximum, config.step);
    return juce::jlimit(allowedMinimum, allowedMaximum, snapped);
}

float RasterHorizontalSliderComponent::normaliseValue(const float plainValue) const {
    return Ui::RasterSliderValueMapping::normaliseValue(plainValue,
                                                        config.minimum,
                                                        config.maximum,
                                                        config.valueMapping);
}

float RasterHorizontalSliderComponent::denormaliseValue(const float normalisedValue) const {
    return Ui::RasterSliderValueMapping::denormaliseValue(normalisedValue,
                                                          config.minimum,
                                                          config.maximum,
                                                          config.valueMapping);
}

juce::String RasterHorizontalSliderComponent::formatValue(const float plainValue) const {
    if (config.formatter != nullptr)
        return config.formatter(plainValue);

    return juce::String(plainValue, 1) + (config.suffix.isNotEmpty() ? " " + config.suffix : juce::String{});
}

std::optional<float> RasterHorizontalSliderComponent::parseValue(const juce::String& text) const {
    if (config.parser != nullptr)
        return config.parser(text);

    return Ui::RasterFilmstrip::parseNumericText(text, config.suffix);
}

int RasterHorizontalSliderComponent::getFrameIndex() const {
    const auto& metrics = theme.metrics.horizontalSlider;
    return Ui::RasterFilmstrip::frameIndexForNormalisedValue(normaliseValue(value),
                                                             metrics.filmstripFrameCount);
}
