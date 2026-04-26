#include "PresetSavePopupContent.h"

#include "PresetPopupLayout.h"
#include "ui/presets/model/PresetActionMessageFormatter.h"
#include "ui/theme/PopupChrome.h"

PresetSavePopupContent::PresetSavePopupContent(const Ui::Theme& themeToUse,
                                               Ui::Presets::PresetUiSnapshot uiSnapshotToUse,
                                               Callbacks callbacksToUse)
    : theme(themeToUse),
      uiSnapshot(std::move(uiSnapshotToUse)),
      callbacks(std::move(callbacksToUse)),
      cancelButton(themeToUse),
      primaryButton(themeToUse),
      statusComponent(themeToUse) {
    jassert(callbacks.onSaveAs != nullptr);
    jassert(callbacks.onOverwrite != nullptr);
    jassert(callbacks.onCloseRequested != nullptr);
    jassert(callbacks.onDismissed != nullptr);
    addAndMakeVisible(nameEditor);
    addAndMakeVisible(cancelButton);
    addAndMakeVisible(primaryButton);
    addAndMakeVisible(statusComponent);

    nameEditor.setText(makeInitialName(), false);
    nameEditor.setSelectAllWhenFocused(true);
    nameEditor.setJustification(juce::Justification::centredLeft);
    nameEditor.setFont(juce::FontOptions(theme.metrics.presetPopup.titleFontHeight, juce::Font::plain));
    nameEditor.applyFontToAllText(nameEditor.getFont(), true);
    nameEditor.setIndents(theme.metrics.presetPopup.saveEditorTextIndentX,
                          theme.metrics.presetPopup.saveEditorTextIndentTop);
    nameEditor.setMultiLine(false);
    nameEditor.setReturnKeyStartsNewLine(false);
    nameEditor.setScrollToShowCursor(true);
    nameEditor.setColour(juce::TextEditor::backgroundColourId, theme.controlSurface);
    nameEditor.setColour(juce::TextEditor::outlineColourId,
                         theme.sectionDividerHighlight.withMultipliedAlpha(theme.metrics.presetPopup.editorOutlineAlpha));
    nameEditor.setColour(juce::TextEditor::focusedOutlineColourId,
                         theme.hardwareMarkingLight.withMultipliedAlpha(theme.metrics.presetPopup.editorFocusOutlineAlpha));
    nameEditor.setColour(juce::TextEditor::textColourId, theme.hardwareMarkingLight);
    nameEditor.setColour(juce::TextEditor::highlightColourId, theme.textSelectionFill);
    nameEditor.setColour(juce::TextEditor::highlightedTextColourId, theme.textSelectionText);
    nameEditor.onTextChange = [this] {
        pendingOverwriteId.reset();
        setStatusMessage({});
        refreshPrimaryButtonState();
    };
    nameEditor.onReturnKey = [this] { invokePrimaryAction(); };
    nameEditor.onEscapeKey = [this] { callbacks.onCloseRequested(); };

    cancelButton.setLabel("Cancel");
    cancelButton.onClick = [this] { callbacks.onCloseRequested(); };

    primaryButton.setStyle(PopupActionButton::Style::primary);
    primaryButton.onClick = [this] { invokePrimaryAction(); };
    refreshPrimaryButtonState();
    setWantsKeyboardFocus(true);
}

PresetSavePopupContent::~PresetSavePopupContent() {
    callbacks.onDismissed();
}

int PresetSavePopupContent::getPreferredWidth() const {
    return theme.metrics.presetPopup.savePopupWidth;
}

int PresetSavePopupContent::getPreferredHeight() const {
    const auto& popupMetrics = theme.metrics.popup;
    const auto statusHeight = statusComponent.getPreferredHeight();
    const auto statusGap = Ui::Presets::getPresetPopupStatusGap(theme, statusHeight);
    return static_cast<int>(popupMetrics.padding * 2
                            + Ui::Presets::getPresetPopupTitleBlockHeight(theme)
                            + theme.metrics.presetPopup.saveEditorHeight
                            + popupMetrics.sectionGap
                            + theme.metrics.presetPopup.saveButtonHeight
                            + static_cast<float>(statusGap)
                            + static_cast<float>(statusHeight));
}

void PresetSavePopupContent::focusInitialControl() {
    nameEditor.grabKeyboardFocus();
}

void PresetSavePopupContent::paint(juce::Graphics& g) {
    Ui::paintPopupShell(g, getLocalBounds().toFloat(), theme);
    g.setColour(theme.axisText.brighter(theme.metrics.presetPopup.saveTitleBrightness));
    const auto& presetPopupMetrics = theme.metrics.presetPopup;
    g.setFont(juce::FontOptions(presetPopupMetrics.titleFontHeight, juce::Font::bold));
    g.drawText(pendingOverwriteId.has_value() ? "Overwrite preset?" : "Save preset",
               getLocalBounds().removeFromTop(Ui::Presets::getPresetPopupTitleBlockHeight(theme)),
               juce::Justification::centred,
               false);
}

void PresetSavePopupContent::resized() {
    const auto& popupMetrics = theme.metrics.popup;
    const auto& presetPopupMetrics = theme.metrics.presetPopup;
    auto bounds = getLocalBounds().reduced(static_cast<int>(popupMetrics.padding));
    bounds.removeFromTop(Ui::Presets::getPresetPopupTitleBlockHeight(theme));
    nameEditor.setBounds(bounds.removeFromTop(presetPopupMetrics.saveEditorHeight));
    bounds.removeFromTop(static_cast<int>(popupMetrics.sectionGap));

    auto buttonRow = Ui::Presets::makePresetPopupButtonRowBounds(Ui::Presets::takePresetPopupButtonArea(bounds, theme), theme);
    cancelButton.setBounds(buttonRow.removeFromLeft(presetPopupMetrics.saveButtonWidth));
    buttonRow.removeFromLeft(presetPopupMetrics.saveButtonGap);
    primaryButton.setBounds(buttonRow.removeFromLeft(presetPopupMetrics.saveButtonWidth));
    const auto statusHeight = statusComponent.getPreferredHeight();
    if (statusHeight > 0) {
        bounds.removeFromTop(presetPopupMetrics.statusTopGap);
        statusComponent.setBounds(bounds.removeFromTop(statusHeight));
    } else {
        statusComponent.setBounds({});
    }
}

bool PresetSavePopupContent::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::escapeKey) {
        callbacks.onCloseRequested();
        return true;
    }

    if (key == juce::KeyPress::returnKey) {
        invokePrimaryAction();
        return true;
    }

    return false;
}

void PresetSavePopupContent::refreshPrimaryButtonState() {
    primaryButton.setLabel(pendingOverwriteId.has_value() ? "Overwrite" : "Save");
    primaryButton.setEnabled(nameEditor.getText().trim().isNotEmpty());
}

void PresetSavePopupContent::invokePrimaryAction() {
    const auto trimmedName = nameEditor.getText().trim();
    if (trimmedName.isEmpty())
        return;

    if (pendingOverwriteId.has_value()) {
        const auto result = callbacks.onOverwrite(*pendingOverwriteId, trimmedName);
        if (!result.succeeded) {
            setStatusMessage(Ui::Presets::makePresetActionMessage(result));
            return;
        }

        setStatusMessage({});
        callbacks.onCloseRequested();
        return;
    }

    const auto matchingPreset = findMatchingPreset(trimmedName);
    if (matchingPreset.has_value()) {
        pendingOverwriteId = matchingPreset->id;
        setStatusMessage({});
        refreshPrimaryButtonState();
        repaint();
        return;
    }

    const auto result = callbacks.onSaveAs(trimmedName);
    if (!result.succeeded) {
        setStatusMessage(Ui::Presets::makePresetActionMessage(result));
        return;
    }

    setStatusMessage({});
    callbacks.onCloseRequested();
}

std::optional<Ui::Presets::PresetDescriptor> PresetSavePopupContent::findMatchingPreset(const juce::String& name) const {
    const auto trimmedName = name.trim();
    if (trimmedName.isEmpty())
        return std::nullopt;

    const auto iterator = std::find_if(uiSnapshot.presets.begin(), uiSnapshot.presets.end(),
                                       [&trimmedName](const Ui::Presets::PresetDescriptor& descriptor) {
                                           return descriptor.name.equalsIgnoreCase(trimmedName);
                                       });
    if (iterator == uiSnapshot.presets.end())
        return std::nullopt;

    if (uiSnapshot.selectedPresetId.has_value() && iterator->id == *uiSnapshot.selectedPresetId)
        return *iterator;

    const auto selectedIterator = std::find_if(uiSnapshot.presets.begin(), uiSnapshot.presets.end(),
                                               [this, &trimmedName](const Ui::Presets::PresetDescriptor& descriptor) {
                                                   return uiSnapshot.selectedPresetId == std::optional<Ui::Presets::PresetId>(descriptor.id)
                                                          && descriptor.name.equalsIgnoreCase(trimmedName);
                                               });
    if (selectedIterator != uiSnapshot.presets.end())
        return *selectedIterator;

    return *iterator;
}

juce::String PresetSavePopupContent::makeInitialName() const {
    if (uiSnapshot.selectedPresetName.isEmpty() || uiSnapshot.selectedPresetName.equalsIgnoreCase("Unsaved"))
        return "Preset";

    return uiSnapshot.selectedPresetName;
}

void PresetSavePopupContent::setStatusMessage(juce::String newMessage) {
    statusComponent.setMessage(std::move(newMessage));
    setSize(getPreferredWidth(), getPreferredHeight());
    resized();
    repaint();
}
