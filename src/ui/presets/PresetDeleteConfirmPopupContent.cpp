#include "PresetDeleteConfirmPopupContent.h"

#include "PresetActionMessageFormatter.h"
#include "PresetPopupLayout.h"
#include "ui/theme/PopupChrome.h"

#include <cmath>

PresetDeleteConfirmPopupContent::PresetDeleteConfirmPopupContent(const Ui::Theme& themeToUse,
                                                                 juce::String presetNameToUse,
                                                                 std::function<PluginPresets::PresetActionResult()> onConfirmToUse,
                                                                 std::function<void()> onDismissToUse)
    : theme(themeToUse),
      presetName(std::move(presetNameToUse)),
      onConfirm(std::move(onConfirmToUse)),
      onDismiss(std::move(onDismissToUse)),
      cancelButton(themeToUse),
      deleteButton(themeToUse),
      statusComponent(themeToUse) {
    addAndMakeVisible(cancelButton);
    addAndMakeVisible(deleteButton);
    addAndMakeVisible(statusComponent);

    cancelButton.setLabel("Cancel");
    deleteButton.setLabel("Delete");
    deleteButton.setStyle(PopupActionButton::Style::primary);

    cancelButton.onClick = [this] {
        if (auto* callout = findParentComponentOfClass<juce::CallOutBox>())
            callout->dismiss();
    };
    deleteButton.onClick = [this] { confirmDelete(); };
    setWantsKeyboardFocus(true);
}

PresetDeleteConfirmPopupContent::~PresetDeleteConfirmPopupContent() {
    if (onDismiss)
        onDismiss();
}

int PresetDeleteConfirmPopupContent::getPreferredWidth() const {
    const auto& popupMetrics = theme.metrics.popup;
    const auto buttonRowWidth = Ui::Presets::getPresetPopupButtonRowWidth(theme);
    auto titleFont = juce::FontOptions(theme.metrics.presetPopup.titleFontHeight, juce::Font::bold);
    const auto titleText = "Delete " + presetName + "?";
    juce::GlyphArrangement titleGlyphs;
    titleGlyphs.addLineOfText(juce::Font(titleFont), titleText, 0.0f, 0.0f);
    const auto titleWidth = static_cast<int>(std::ceil(titleGlyphs.getBoundingBox(0, -1, true).getWidth()));
    const auto contentWidth = juce::jmax(buttonRowWidth, titleWidth);
    return static_cast<int>(popupMetrics.padding * 2) + contentWidth;
}

int PresetDeleteConfirmPopupContent::getPreferredHeight() const {
    const auto& popupMetrics = theme.metrics.popup;
    const auto statusHeight = statusComponent.getPreferredHeight();
    const auto statusGap = Ui::Presets::getPresetPopupStatusGap(theme, statusHeight);
    return static_cast<int>(popupMetrics.padding * 2
                            + Ui::Presets::getPresetPopupTitleBlockHeight(theme)
                            + theme.metrics.presetPopup.saveButtonHeight
                            + static_cast<float>(statusGap)
                            + static_cast<float>(statusHeight));
}

void PresetDeleteConfirmPopupContent::paint(juce::Graphics& g) {
    Ui::paintPopupShell(g, getLocalBounds().toFloat(), theme);
    const auto& presetPopupMetrics = theme.metrics.presetPopup;
    g.setColour(theme.axisText.brighter(presetPopupMetrics.confirmTitleBrightness));
    g.setFont(juce::FontOptions(presetPopupMetrics.titleFontHeight, juce::Font::bold));
    g.drawText("Delete " + presetName + "?",
               getLocalBounds().removeFromTop(Ui::Presets::getPresetPopupTitleBlockHeight(theme)),
               juce::Justification::centred,
               true);
}

void PresetDeleteConfirmPopupContent::resized() {
    const auto& popupMetrics = theme.metrics.popup;
    const auto& presetPopupMetrics = theme.metrics.presetPopup;
    auto bounds = getLocalBounds().reduced(static_cast<int>(popupMetrics.padding));
    bounds.removeFromTop(Ui::Presets::getPresetPopupTitleBlockHeight(theme));

    auto buttonRow = Ui::Presets::makePresetPopupButtonRowBounds(Ui::Presets::takePresetPopupButtonArea(bounds, theme), theme);
    cancelButton.setBounds(buttonRow.removeFromLeft(presetPopupMetrics.saveButtonWidth));
    buttonRow.removeFromLeft(presetPopupMetrics.saveButtonGap);
    deleteButton.setBounds(buttonRow.removeFromLeft(presetPopupMetrics.saveButtonWidth));
    const auto statusHeight = statusComponent.getPreferredHeight();
    if (statusHeight > 0) {
        bounds.removeFromTop(presetPopupMetrics.statusTopGap);
        statusComponent.setBounds(bounds.removeFromTop(statusHeight));
    } else {
        statusComponent.setBounds({});
    }
}

bool PresetDeleteConfirmPopupContent::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::escapeKey) {
        if (auto* callout = findParentComponentOfClass<juce::CallOutBox>())
            callout->dismiss();
        return true;
    }

    if (key == juce::KeyPress::returnKey) {
        confirmDelete();
        return true;
    }

    return false;
}

void PresetDeleteConfirmPopupContent::confirmDelete() {
    const auto result = onConfirm ? onConfirm()
                                  : PluginPresets::PresetActionResult::ok();
    if (!result.succeeded) {
        setStatusMessage(Ui::Presets::makePresetActionMessage(result));
        return;
    }

    setStatusMessage({});

    if (auto* callout = findParentComponentOfClass<juce::CallOutBox>())
        callout->dismiss();
}

void PresetDeleteConfirmPopupContent::setStatusMessage(juce::String newMessage) {
    statusComponent.setMessage(std::move(newMessage));
    setSize(getPreferredWidth(), getPreferredHeight());
    resized();
    repaint();
}
