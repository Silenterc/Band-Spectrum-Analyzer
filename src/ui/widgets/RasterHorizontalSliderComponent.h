#pragma once

#include <functional>
#include <optional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "RasterFilmstrip.h"
#include "RasterSliderValueMapping.h"
#include "ui/theme/UiTheme.h"

class RasterHorizontalSliderComponent final : public juce::Component {
public:
    struct Config {
        juce::String label;
        float minimum = 0.0f;
        float maximum = 1.0f;
        float step = 0.0f;
        juce::String suffix;
        Ui::SliderValueMapping valueMapping = Ui::SliderValueMapping::linear;
        std::function<juce::String(float)> formatter;
        std::function<std::optional<float>(const juce::String&)> parser;
    };

    explicit RasterHorizontalSliderComponent(const Ui::Theme& themeToUse);
    ~RasterHorizontalSliderComponent() override;

    void setConfig(Config newConfig);
    void setValue(float newValue, juce::NotificationType notificationType = juce::dontSendNotification);
    /** Tightens the interactive clamp window without changing the control's visual range mapping. */
    void setAllowedRange(float minimum, float maximum);
    [[nodiscard]] float getValue() const;
    [[nodiscard]] juce::Rectangle<int> getPreferredBounds() const;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& key) override;

    std::function<void()> onGestureStart;
    std::function<void(float)> onValueChanged;
    std::function<void()> onGestureEnd;

private:
    class InlineValueEditor final : public juce::TextEditor {
    public:
        explicit InlineValueEditor(RasterHorizontalSliderComponent& ownerToUse);

        void focusLost(FocusChangeType cause) override;

    private:
        RasterHorizontalSliderComponent& owner;
    };

    class OutsideClickListener final : public juce::MouseListener {
    public:
        explicit OutsideClickListener(RasterHorizontalSliderComponent& ownerToUse);

        void mouseDown(const juce::MouseEvent& event) override;

    private:
        RasterHorizontalSliderComponent& owner;
    };

    void configureEditor();
    void beginValueEdit();
    void commitValueEdit();
    void cancelValueEdit();
    void attachOutsideClickListener();
    void detachOutsideClickListener();
    [[nodiscard]] bool isPointInsideValueEditor(const juce::MouseEvent& event) const;
    void beginDrag(const juce::MouseEvent& event);
    void updateDrag(const juce::MouseEvent& event);
    void endDrag();
    void updateMouseCursor(juce::Point<int> position);
    [[nodiscard]] bool isInteractivePosition(juce::Point<int> position) const;
    void updateLayout();
    void updateValueEditorText();
    void setValueInternal(float newValue, juce::NotificationType notificationType);
    [[nodiscard]] float snapAndClamp(float plainValue) const;
    [[nodiscard]] float normaliseValue(float plainValue) const;
    [[nodiscard]] float denormaliseValue(float normalisedValue) const;
    [[nodiscard]] juce::String formatValue(float plainValue) const;
    [[nodiscard]] std::optional<float> parseValue(const juce::String& text) const;
    [[nodiscard]] int getFrameIndex() const;

    const Ui::Theme& theme;
    Config config;
    float value = 0.0f;
    float allowedMinimum = 0.0f;
    float allowedMaximum = 1.0f;
    float dragStartNormalised = 0.0f;
    float dragStartX = 0.0f;
    bool dragging = false;
    bool cancellingEdit = false;
    bool outsideClickListenerAttached = false;
    juce::Rectangle<int> labelBounds;
    juce::Rectangle<int> sliderBounds;
    juce::Rectangle<int> valueBounds;
    InlineValueEditor valueEditor;
    OutsideClickListener outsideClickListener;
};
