#include "RasterKnobComponent.h"

#include <cmath>

#include "ui/theme/UiRasterAssets.h"

namespace {
    Ui::RasterFilmstripSpec getKnobFilmstripSpec(const Ui::KnobMetrics& metrics) {
        return {
            .frameCount = metrics.filmstripFrameCount,
            .frameWidth = metrics.filmstripFrameWidth,
            .frameHeight = metrics.filmstripFrameHeight,
            .orientation = Ui::FilmstripOrientation::vertical
        };
    }
}

RasterKnobComponent::InlineValueEditor::InlineValueEditor(RasterKnobComponent& ownerToUse)
    : owner(ownerToUse) {
}

void RasterKnobComponent::InlineValueEditor::focusLost(const FocusChangeType cause) {
    juce::TextEditor::focusLost(cause);
    owner.commitValueEdit();
}

RasterKnobComponent::OutsideClickListener::OutsideClickListener(RasterKnobComponent& ownerToUse)
    : owner(ownerToUse) {
}

void RasterKnobComponent::OutsideClickListener::mouseDown(const juce::MouseEvent& event) {
    if (owner.valueEditor.isVisible() && !owner.isPointInsideValueEditor(event))
        owner.commitValueEdit();
}

RasterKnobComponent::RasterKnobComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      valueEditor(*this),
      outsideClickListener(*this) {
    setWantsKeyboardFocus(true);
    setMouseCursor(juce::MouseCursor::NormalCursor);
    addChildComponent(valueEditor);
    configureEditor();
    updateLayout();
}

RasterKnobComponent::~RasterKnobComponent() {
    detachOutsideClickListener();
}

void RasterKnobComponent::setConfig(Config newConfig) {
    config = std::move(newConfig);
    allowedMinimum = config.minimum;
    allowedMaximum = config.maximum;
    value = snapAndClamp(value);
    updateValueEditorText();
    repaint();
}

void RasterKnobComponent::setValue(const float newValue, const juce::NotificationType notificationType) {
    setValueInternal(newValue, notificationType);
}

void RasterKnobComponent::setAllowedRange(const float minimum, const float maximum) {
    allowedMinimum = minimum;
    allowedMaximum = maximum;
    setValueInternal(value, juce::dontSendNotification);
}

float RasterKnobComponent::getValue() const {
    return value;
}

juce::Rectangle<int> RasterKnobComponent::getPreferredBounds() const {
    return {0, 0, theme.metrics.knob.width, theme.metrics.knob.height};
}

void RasterKnobComponent::paint(juce::Graphics& g) {
    const auto& metrics = theme.metrics.knob;
    g.setColour(theme.hardwareMarkingLight);
    g.setFont(juce::FontOptions(metrics.labelFontHeight).withStyle("Bold"));
    g.drawText(config.label, labelBounds, juce::Justification::centred, false);

    const auto& scaleImage = Ui::getControlRasterAsset(Ui::ControlRasterAssetId::knobSmallScale);
    if (scaleImage.isValid())
        Ui::drawAssetWithin(g, scaleImage, scaleBounds);

    const auto& knobImage = Ui::getControlRasterAsset(Ui::ControlRasterAssetId::knobSmallFilmstrip);
    Ui::RasterFilmstrip::drawFrame(g, knobImage, getKnobFilmstripSpec(metrics), getFrameIndex(), knobBounds);

    const auto& valueImage = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::textBox);
    if (valueImage.isValid())
        Ui::drawAssetWithin(g, valueImage, valueBounds);

    if (!valueEditor.isVisible()) {
        g.setColour(theme.hardwareMarkingLight);
        g.setFont(juce::FontOptions(metrics.valueFontHeight).withStyle("Bold"));
        g.drawText(formatValue(value), valueBounds, juce::Justification::centred, false);
    }
}

void RasterKnobComponent::resized() {
    updateLayout();
}

void RasterKnobComponent::mouseDown(const juce::MouseEvent& event) {
    if (valueBounds.contains(event.getPosition())) {
        beginValueEdit();
        return;
    }

    if (knobBounds.contains(event.getPosition()) || scaleBounds.contains(event.getPosition()))
        beginDrag(event);
}

void RasterKnobComponent::mouseMove(const juce::MouseEvent& event) {
    updateMouseCursor(event.getPosition());
}

void RasterKnobComponent::mouseExit(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void RasterKnobComponent::mouseDrag(const juce::MouseEvent& event) {
    if (dragging)
        updateDrag(event);
}

void RasterKnobComponent::mouseUp(const juce::MouseEvent& event) {
    juce::ignoreUnused(event);
    if (dragging)
        endDrag();
}

bool RasterKnobComponent::keyPressed(const juce::KeyPress& key) {
    const auto increment = key.getModifiers().isShiftDown()
                               ? config.step * theme.metrics.knob.keyboardStepMultiplier
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

void RasterKnobComponent::configureEditor() {
    valueEditor.setSelectAllWhenFocused(true);
    valueEditor.setJustification(juce::Justification::centred);
    valueEditor.setMultiLine(false);
    valueEditor.setReturnKeyStartsNewLine(false);
    valueEditor.setScrollToShowCursor(true);
    valueEditor.setFont(juce::FontOptions(theme.metrics.knob.valueFontHeight, juce::Font::bold));
    valueEditor.applyFontToAllText(valueEditor.getFont(), true);
    valueEditor.setIndents(theme.metrics.knob.valueEditorTextIndentX,
                           theme.metrics.knob.valueEditorTextIndentTop);
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

void RasterKnobComponent::beginValueEdit() {
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

void RasterKnobComponent::commitValueEdit() {
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

void RasterKnobComponent::cancelValueEdit() {
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

void RasterKnobComponent::attachOutsideClickListener() {
    if (outsideClickListenerAttached)
        return;

    juce::Desktop::getInstance().addGlobalMouseListener(&outsideClickListener);
    outsideClickListenerAttached = true;
}

void RasterKnobComponent::detachOutsideClickListener() {
    if (!outsideClickListenerAttached)
        return;

    juce::Desktop::getInstance().removeGlobalMouseListener(&outsideClickListener);
    outsideClickListenerAttached = false;
}

bool RasterKnobComponent::isPointInsideValueEditor(const juce::MouseEvent& event) const {
    if (event.eventComponent == &valueEditor || valueEditor.isParentOf(event.eventComponent))
        return true;

    auto screenPoint = event.getScreenPosition();
    if (event.eventComponent != nullptr)
        screenPoint = event.eventComponent->localPointToGlobal(event.getPosition());

    return valueEditor.getScreenBounds().contains(screenPoint);
}

void RasterKnobComponent::beginDrag(const juce::MouseEvent& event) {
    dragging = true;
    dragStartValue = value;
    dragStartY = event.position.y;
    grabKeyboardFocus();
    if (onGestureStart != nullptr)
        onGestureStart();
}

void RasterKnobComponent::updateDrag(const juce::MouseEvent& event) {
    const auto range = config.maximum - config.minimum;
    if (range <= 0.0f)
        return;

    const auto deltaNormalised = (dragStartY - event.position.y) / juce::jmax(1.0f, theme.metrics.knob.dragPixelsForFullRange);
    setValueInternal(dragStartValue + deltaNormalised * range, juce::sendNotification);
}

void RasterKnobComponent::endDrag() {
    dragging = false;
    if (onGestureEnd != nullptr)
        onGestureEnd();
}

void RasterKnobComponent::updateMouseCursor(const juce::Point<int> position) {
    setMouseCursor(isInteractivePosition(position) ? juce::MouseCursor::PointingHandCursor
                                                   : juce::MouseCursor::NormalCursor);
}

bool RasterKnobComponent::isInteractivePosition(const juce::Point<int> position) const {
    return valueBounds.contains(position) || knobBounds.contains(position) || scaleBounds.contains(position);
}

void RasterKnobComponent::updateLayout() {
    const auto& metrics = theme.metrics.knob;
    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        bounds = getPreferredBounds();

    labelBounds = bounds.removeFromTop(metrics.labelHeight);
    bounds.removeFromTop(metrics.labelToScaleGap);

    const auto scaleAndKnobHeight = juce::jmax(metrics.scaleHeight, metrics.knobSide);
    const auto scaleAndKnobBounds = bounds.removeFromTop(scaleAndKnobHeight);
    const auto centre = scaleAndKnobBounds.getCentre();
    scaleBounds = juce::Rectangle<int>(metrics.scaleWidth, metrics.scaleHeight).withCentre(centre.translated(0, metrics.scaleOffsetY));
    knobBounds = juce::Rectangle<int>(metrics.knobSide, metrics.knobSide).withCentre(centre);

    bounds.removeFromTop(metrics.scaleToValueGap);
    valueBounds = juce::Rectangle<int>(metrics.valueWidth, metrics.valueHeight).withCentre({getLocalBounds().getCentreX(),
                                                                                            bounds.getY() + metrics.valueHeight / 2});
    valueEditor.setBounds(valueBounds);
}

void RasterKnobComponent::updateValueEditorText() {
    valueEditor.setText(formatValue(value), false);
}

void RasterKnobComponent::setValueInternal(const float newValue, const juce::NotificationType notificationType) {
    const auto nextValue = snapAndClamp(newValue);
    if (juce::approximatelyEqual(value, nextValue))
        return;

    value = nextValue;
    updateValueEditorText();
    repaint();
    if (notificationType != juce::dontSendNotification && onValueChanged != nullptr)
        onValueChanged(value);
}

float RasterKnobComponent::snapAndClamp(const float plainValue) const {
    const auto snapped = Ui::RasterFilmstrip::snapValue(plainValue, config.minimum, config.maximum, config.step);
    return juce::jlimit(allowedMinimum, allowedMaximum, snapped);
}

juce::String RasterKnobComponent::formatValue(const float plainValue) const {
    if (config.formatter != nullptr)
        return config.formatter(plainValue);

    return juce::String(plainValue, 1) + (config.suffix.isNotEmpty() ? " " + config.suffix : juce::String{});
}

std::optional<float> RasterKnobComponent::parseValue(const juce::String &text) const {
    if (config.parser != nullptr)
        return config.parser(text);

    return Ui::RasterFilmstrip::parseNumericText(text, config.suffix);
}

int RasterKnobComponent::getFrameIndex() const {
    const auto& metrics = theme.metrics.knob;
    return Ui::RasterFilmstrip::frameIndexForNormalisedValue(
        Ui::RasterFilmstrip::normaliseValue(value, config.minimum, config.maximum),
        metrics.filmstripFrameCount);
}
