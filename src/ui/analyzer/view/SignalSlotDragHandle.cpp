#include "SignalSlotDragHandle.h"

SignalSlotDragHandle::SignalSlotDragHandle(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

void SignalSlotDragHandle::setDragged(const bool isDraggedValue) {
    dragged = isDraggedValue;
    repaint();
}

void SignalSlotDragHandle::paint(juce::Graphics &g) {
    const auto bounds = getLocalBounds().toFloat();
    g.setColour((hovered || dragged) ? theme.controlSurfaceHover : theme.controlSurface);
    g.fillRoundedRectangle(bounds, theme.metrics.slot.buttonCornerRadius);
    g.setColour(theme.controlBorder);
    g.drawRoundedRectangle(bounds.reduced(0.5f), theme.metrics.slot.buttonCornerRadius, 1.0f);

    g.setColour(theme.subtleText);
    const auto gripX = bounds.getCentreX() - theme.metrics.slot.gripWidth * 0.5f;
    const auto gripY = bounds.getCentreY() - theme.metrics.slot.gripHeight * 0.5f;
    const auto columnStep = theme.metrics.slot.gripWidth - theme.metrics.slot.gripDotDiameter;
    const auto rowStep = (theme.metrics.slot.gripHeight - theme.metrics.slot.gripDotDiameter) * 0.5f;
    for (int dotColumn = 0; dotColumn < 2; ++dotColumn) {
        for (int dotRow = 0; dotRow < 3; ++dotRow) {
            const auto x = gripX + static_cast<float>(dotColumn) * columnStep;
            const auto y = gripY + static_cast<float>(dotRow) * rowStep;
            g.fillEllipse(x, y, theme.metrics.slot.gripDotDiameter, theme.metrics.slot.gripDotDiameter);
        }
    }
}

void SignalSlotDragHandle::mouseEnter(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hovered = true;
    repaint();
}

void SignalSlotDragHandle::mouseExit(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    hovered = false;
    repaint();
}

void SignalSlotDragHandle::mouseDown(const juce::MouseEvent &event) {
    mouseDownPosition = event.position;
    trackingDrag = false;
}

void SignalSlotDragHandle::mouseDrag(const juce::MouseEvent &event) {
    const auto delta = event.position - mouseDownPosition;
    if (!trackingDrag && delta.getDistanceFromOrigin() > 6.0f) {
        trackingDrag = true;
        if (onDragStarted)
            onDragStarted(getParentRelativeX(event));
    }

    if (trackingDrag && onDragged)
        onDragged(getParentRelativeX(event));
}

void SignalSlotDragHandle::mouseUp(const juce::MouseEvent &event) {
    if (trackingDrag && onDragEnded)
        onDragEnded(getParentRelativeX(event));

    trackingDrag = false;
}

float SignalSlotDragHandle::getParentRelativeX(const juce::MouseEvent &event) const {
    if (auto *parent = const_cast<juce::Component *>(getParentComponent()))
        return event.getEventRelativeTo(parent).position.x;

    return event.position.x;
}
