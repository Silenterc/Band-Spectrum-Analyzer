#include "ui/widgets/BrandLogoComponent.h"

#include <BinaryData.h>

namespace {
    juce::Drawable* getLogoTemplate() {
        static auto drawable = juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize);
        return drawable.get();
    }

    float getLogoAspectRatio() {
        const auto* drawable = getLogoTemplate();
        if (drawable == nullptr)
            return 1.0f;

        const auto drawableBounds = drawable->getDrawableBounds();
        if (drawableBounds.getHeight() <= 0.0f)
            return 1.0f;

        return drawableBounds.getWidth() / drawableBounds.getHeight();
    }
}

BrandLogoComponent::BrandLogoComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse) {
    setInterceptsMouseClicks(false, false);
}

void BrandLogoComponent::paint(juce::Graphics& g) {
    drawLogo(g, logoBounds);
}

void BrandLogoComponent::resized() {
    auto contentBounds = getLocalBounds();
    contentBounds.removeFromLeft(theme.metrics.presetHeader.logoLeftInset);
    logoBounds = getLogoBounds(contentBounds, theme);
}

void BrandLogoComponent::drawLogo(juce::Graphics& g, const juce::Rectangle<int>& bounds) {
    if (bounds.isEmpty())
        return;

    auto* templateDrawable = getLogoTemplate();
    if (templateDrawable == nullptr)
        return;

    auto drawable = std::unique_ptr<juce::Drawable>(templateDrawable->createCopy());
    if (drawable == nullptr)
        return;

    drawable->drawWithin(g,
                         bounds.toFloat(),
                         juce::RectanglePlacement(juce::RectanglePlacement::xLeft
                                                  | juce::RectanglePlacement::yMid),
                         1.0f);
}

juce::Rectangle<int> BrandLogoComponent::getLogoBounds(const juce::Rectangle<int> availableBounds,
                                                       const Ui::Theme& theme) {
    const auto logoHeight = juce::jmax(1, theme.metrics.presetHeader.logoHeight);
    const auto logoWidth = juce::jmax(1, juce::roundToInt(static_cast<float>(logoHeight) * getLogoAspectRatio()));
    return juce::Rectangle<int>(logoWidth, logoHeight)
        .withX(availableBounds.getX())
        .withCentre({availableBounds.getX() + logoWidth / 2, availableBounds.getCentreY()})
        .translated(0, theme.metrics.presetHeader.logoOpticalYOffset);
}
