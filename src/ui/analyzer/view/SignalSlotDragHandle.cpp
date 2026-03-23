#include "SignalSlotDragHandle.h"

#include "../../UiIcons.h"
#include "../../UiRasterAssets.h"

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
    const auto &padImage = (hovered || dragged) ? cachedOnImage : cachedOffImage;
    if (padImage.isValid())
        g.drawImageAt(padImage, 0, 0);

    const auto iconColour = (hovered || dragged) ? theme.hardwareMarkingDark : theme.hardwareMarkingLight;
    Ui::drawGripIcon(g, bounds.reduced(bounds.getWidth() * 0.28f), iconColour);
}

void SignalSlotDragHandle::resized() {
    rebuildCachedImages();
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

void SignalSlotDragHandle::rebuildCachedImages() {
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty()) {
        cachedOffImage = {};
        cachedOnImage = {};
        return;
    }

    cachedOffImage = Ui::getRasterAsset(Ui::RasterAssetId::padOff).rescaled(bounds.getWidth(),
                                                                             bounds.getHeight(),
                                                                             juce::Graphics::highResamplingQuality);
    cachedOnImage = Ui::getRasterAsset(Ui::RasterAssetId::padOn).rescaled(bounds.getWidth(),
                                                                           bounds.getHeight(),
                                                                           juce::Graphics::highResamplingQuality);
}
