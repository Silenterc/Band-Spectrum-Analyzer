#include "CalloutPresenter.h"

namespace Ui {
    CalloutPresenter::CalloutPresenter(juce::LookAndFeel& lookAndFeelToUse)
        : lookAndFeel(lookAndFeelToUse) {
    }

    CalloutPresenter::~CalloutPresenter() {
        dismiss();
    }

    bool CalloutPresenter::isShowing() const {
        return activeCallout != nullptr;
    }

    juce::CallOutBox* CalloutPresenter::getActiveCallout() const {
        return activeCallout.getComponent();
    }

    juce::CallOutBox* CalloutPresenter::launch(std::unique_ptr<juce::Component> content,
                                                const juce::Rectangle<int> anchorBounds,
                                                juce::Component& parentComponent) {
        jassert(content != nullptr);
        if (content == nullptr)
            return nullptr;

        dismiss();
        auto& callout = juce::CallOutBox::launchAsynchronously(std::move(content), anchorBounds, &parentComponent);
        callout.setLookAndFeel(&lookAndFeel);
        callout.lookAndFeelChanged();
        callout.setDismissalMouseClicksAreAlwaysConsumed(false);
        activeCallout = &callout;
        return &callout;
    }

    void CalloutPresenter::dismiss() {
        if (activeCallout == nullptr)
            return;

        activeCallout->setLookAndFeel(nullptr);
        activeCallout->dismiss();
        activeCallout = nullptr;
    }

    void CalloutPresenter::forget() {
        activeCallout = nullptr;
    }

    juce::Component* findCalloutParentComponent(const juce::Component& component) {
        auto& mutableComponent = const_cast<juce::Component&>(component);
        if (auto* parentComponent = mutableComponent.getTopLevelComponent())
            return parentComponent;

        return mutableComponent.getParentComponent();
    }

    juce::Rectangle<int> makePointCalloutAnchor(const juce::Rectangle<int> bounds,
                                                const bool useBottomEdge) {
        return {bounds.getCentreX(), useBottomEdge ? bounds.getBottom() : bounds.getY(), 1, 1};
    }
}
