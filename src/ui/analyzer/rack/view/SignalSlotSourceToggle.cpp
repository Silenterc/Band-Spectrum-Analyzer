#include "SignalSlotSourceToggle.h"

#include "ui/theme/UiRasterAssets.h"
#include "ui/analyzer/rack/model/SignalSlotOptions.h"

SignalSlotSourceToggle::SignalSlotSourceToggle(const Ui::Theme &themeToUse)
    : theme(themeToUse) {
}

void SignalSlotSourceToggle::setState(const Analyzer::SignalSource sourceToUse, const bool sidechainAvailableToUse) {
    source = sourceToUse;
    sidechainAvailable = sidechainAvailableToUse;
    rebuildCachedSwitch();
    repaint();
}

void SignalSlotSourceToggle::paint(juce::Graphics &g) {
    const auto &slotMetrics = theme.metrics.slot;
    const auto mainActive = source == Analyzer::SignalSource::main;
    const auto sidechainActive = source == Analyzer::SignalSource::sidechain;

    const auto switchBounds = getSwitchBounds().getSmallestIntegerContainer();
    if (cachedSwitchImage.isValid())
        g.drawImageAt(cachedSwitchImage, switchBounds.getX(), switchBounds.getY());

    g.setFont(slotMetrics.hintFontHeight + slotMetrics.sourceToggleFontDelta);
    g.setColour(mainActive ? theme.hardwareMarkingLight : theme.axisText);
    g.drawText(Ui::getSignalSourceLabel(Analyzer::SignalSource::main), getTopLabelBounds(), juce::Justification::centred);

    const auto sidechainColour = sidechainAvailable
                                     ? (sidechainActive ? theme.hardwareMarkingLight : theme.axisText)
                                     : theme.subtleText.withMultipliedAlpha(0.7f);
    g.setColour(sidechainColour);
    g.drawText(Ui::getSignalSourceLabel(Analyzer::SignalSource::sidechain), getBottomLabelBounds(), juce::Justification::centred);
}

void SignalSlotSourceToggle::resized() {
    rebuildCachedSwitch();
}

void SignalSlotSourceToggle::mouseMove(const juce::MouseEvent &) {
    setMouseCursor(sidechainAvailable ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
}

void SignalSlotSourceToggle::mouseUp(const juce::MouseEvent &event) {
    if (event.mouseWasDraggedSinceMouseDown() || onSourceSelected == nullptr)
        return;

    if (source == Analyzer::SignalSource::main) {
        if (sidechainAvailable)
            onSourceSelected(Analyzer::SignalSource::sidechain);
    } else {
        onSourceSelected(Analyzer::SignalSource::main);
    }
}

juce::Rectangle<float> SignalSlotSourceToggle::getSwitchBounds() const {
    auto bounds = getLocalBounds().toFloat();
    const auto &slotMetrics = theme.metrics.slot;
    const auto labelHeight = juce::jmin(slotMetrics.sourceToggleMaxLabelHeight,
                                        bounds.getHeight() * slotMetrics.sourceToggleLabelHeightFraction);
    bounds.removeFromTop(labelHeight);
    bounds.removeFromBottom(labelHeight);
    bounds = bounds.reduced(slotMetrics.sourceToggleSwitchInsetX, 0.0f);

    const auto &sourceImage = Ui::getAnalyzerRasterAsset(Ui::AnalyzerRasterAssetId::switchUp);
    const auto logicalWidth = static_cast<float>(sourceImage.getWidth()) / theme.metrics.assets.rasterScale;
    const auto logicalHeight = static_cast<float>(sourceImage.getHeight()) / theme.metrics.assets.rasterScale;
    const auto fitScale = juce::jmin(bounds.getWidth() / logicalWidth, bounds.getHeight() / logicalHeight);
    const auto clampedScale = juce::jlimit(0.0f, 1.0f, fitScale);
    const auto switchWidth = juce::jmax(1.0f, logicalWidth * clampedScale);
    const auto switchHeight = juce::jmax(1.0f, logicalHeight * clampedScale);
    return juce::Rectangle<float>(switchWidth, switchHeight).withCentre(bounds.getCentre());
}

juce::Rectangle<int> SignalSlotSourceToggle::getTopLabelBounds() const {
    auto bounds = getLocalBounds();
    bounds.setHeight(static_cast<int>(std::round(getSwitchBounds().getY())));
    return bounds;
}

juce::Rectangle<int> SignalSlotSourceToggle::getBottomLabelBounds() const {
    auto bounds = getLocalBounds();
    bounds.removeFromTop(static_cast<int>(std::round(getSwitchBounds().getBottom())));
    return bounds;
}

void SignalSlotSourceToggle::rebuildCachedSwitch() {
    const auto switchBounds = getSwitchBounds().getSmallestIntegerContainer();
    if (switchBounds.isEmpty()) {
        cachedSwitchImage = {};
        return;
    }

    const auto &sourceImage = Ui::getAnalyzerRasterAsset(source == Analyzer::SignalSource::sidechain
                                                             ? Ui::AnalyzerRasterAssetId::switchDown
                                                             : Ui::AnalyzerRasterAssetId::switchUp);
    cachedSwitchImage = juce::Image(juce::Image::ARGB, switchBounds.getWidth(), switchBounds.getHeight(), true);
    juce::Graphics graphics(cachedSwitchImage);
    graphics.drawImage(sourceImage,
                       0,
                       0,
                       switchBounds.getWidth(),
                       switchBounds.getHeight(),
                       0,
                       0,
                       sourceImage.getWidth(),
                       sourceImage.getHeight());
}
