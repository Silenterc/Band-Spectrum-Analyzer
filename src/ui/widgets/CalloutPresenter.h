#pragma once

#include <memory>

#include <juce_gui_basics/juce_gui_basics.h>

namespace Ui {
    class CalloutPresenter final {
    public:
        explicit CalloutPresenter(juce::LookAndFeel& lookAndFeelToUse);
        ~CalloutPresenter();

        [[nodiscard]] bool isShowing() const;
        [[nodiscard]] juce::CallOutBox* getActiveCallout() const;

        juce::CallOutBox* launch(std::unique_ptr<juce::Component> content,
                                 juce::Rectangle<int> anchorBounds,
                                 juce::Component& parentComponent);
        void dismiss();
        void forget();

    private:
        juce::LookAndFeel& lookAndFeel;
        juce::Component::SafePointer<juce::CallOutBox> activeCallout;
    };

    [[nodiscard]] juce::Component* findCalloutParentComponent(const juce::Component& component);
    [[nodiscard]] juce::Rectangle<int> makePointCalloutAnchor(juce::Rectangle<int> bounds,
                                                              bool useBottomEdge);
}
