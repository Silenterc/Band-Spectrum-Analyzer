#include "PresetHeaderComponent.h"

#include <BinaryData.h>

#include "ui/theme/UiRasterAssets.h"

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

    void drawLogo(juce::Graphics& g, const juce::Rectangle<int>& bounds) {
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
}

PresetHeaderComponent::PresetHeaderComponent(const Ui::Theme& themeToUse)
    : theme(themeToUse),
      previousButton(themeToUse),
      nextButton(themeToUse),
      resetButton(themeToUse),
      saveButton(themeToUse) {
    previousButton.setIcon(Ui::IconId::left);
    nextButton.setIcon(Ui::IconId::right);
    resetButton.setIcon(Ui::IconId::reset);
    saveButton.setIcon(Ui::IconId::save);

    previousButton.setTooltip("Previous preset");
    nextButton.setTooltip("Next preset");
    resetButton.setTooltip("Reset preset");
    saveButton.setTooltip("Save preset");

    for (auto* button : { &previousButton, &nextButton, &resetButton, &saveButton }) {
        button->setWantsKeyboardFocus(false);
        button->setScaleMultiplier(theme.metrics.presetHeader.buttonScale);
        button->setIconScaleMultiplier(theme.metrics.presetHeader.buttonIconScaleMultiplier);
        addAndMakeVisible(button);
    }
}

void PresetHeaderComponent::paint(juce::Graphics& g) {
    drawLogo(g, logoBounds);

    if (cachedTextBoxImage.isValid())
        g.drawImageAt(cachedTextBoxImage, textBoxBounds.getX(), textBoxBounds.getY());

    g.setColour(theme.hardwareMarkingLight);
    g.setFont(juce::FontOptions(theme.metrics.presetHeader.labelFontHeight).withStyle("Bold"));
    g.drawText("Default", textBoxBounds, juce::Justification::centred, false);
}

void PresetHeaderComponent::resized() {
    const auto& metrics = theme.metrics.presetHeader;
    const auto buttonSide = previousButton.getPreferredSideLength();
    const auto textBoxLogicalBounds = getScaledTextBoxBounds(getLocalBounds());
    const auto rowWidth = buttonSide * 4
                          + textBoxLogicalBounds.getWidth()
                          + metrics.displayGap * 2
                          + metrics.groupGap
                          + metrics.actionGap;
    auto contentBounds = getLocalBounds();
    contentBounds.removeFromLeft(metrics.logoLeftInset);
    logoBounds = getLogoBounds(contentBounds);

    auto rowBounds = juce::Rectangle<int>(rowWidth, getHeight())
                         .withY(getLocalBounds().getY())
                         .withX(logoBounds.isEmpty() ? contentBounds.getX() : logoBounds.getRight() + metrics.logoGap);

    previousButton.setBounds(rowBounds.removeFromLeft(buttonSide));
    rowBounds.removeFromLeft(metrics.displayGap);
    textBoxBounds = getScaledTextBoxBounds(rowBounds.removeFromLeft(textBoxLogicalBounds.getWidth()));
    rowBounds.removeFromLeft(metrics.displayGap);
    nextButton.setBounds(rowBounds.removeFromLeft(buttonSide));
    rowBounds.removeFromLeft(metrics.groupGap);
    resetButton.setBounds(rowBounds.removeFromLeft(buttonSide));
    rowBounds.removeFromLeft(metrics.actionGap);
    saveButton.setBounds(rowBounds.removeFromLeft(buttonSide));

    if (textBoxBounds.isEmpty()) {
        cachedTextBoxImage = {};
        return;
    }

    cachedTextBoxImage = Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::textBox).rescaled(
        textBoxBounds.getWidth(),
        textBoxBounds.getHeight(),
        juce::Graphics::highResamplingQuality);
}

juce::Rectangle<int> PresetHeaderComponent::getScaledTextBoxBounds(const juce::Rectangle<int> availableBounds) const {
    return Ui::getScaledAssetBoundsWithin(Ui::getSharedRasterAsset(Ui::SharedRasterAssetId::textBox),
                                          theme.metrics.assets.rasterScale,
                                          availableBounds,
                                          theme.metrics.presetHeader.textBoxScale);
}

juce::Rectangle<int> PresetHeaderComponent::getLogoBounds(const juce::Rectangle<int> availableBounds) const {
    const auto logoHeight = juce::jmax(1, theme.metrics.presetHeader.logoHeight);
    const auto logoWidth = juce::jmax(1, juce::roundToInt(static_cast<float>(logoHeight) * getLogoAspectRatio()));
    return juce::Rectangle<int>(logoWidth, logoHeight)
        .withX(availableBounds.getX())
        .withCentre({availableBounds.getX() + logoWidth / 2, availableBounds.getCentreY()});
}
