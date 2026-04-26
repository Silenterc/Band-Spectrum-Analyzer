#pragma once

#include "ui/editor/state/EditorPresentationState.h"

class EditorPresentationStateSource {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void editorPresentationStateChanged(const Ui::EditorPresentationState &state) = 0;
    };

    virtual ~EditorPresentationStateSource() = default;

    virtual Ui::EditorPresentationState getEditorPresentationState() const = 0;
    virtual void addEditorPresentationStateListener(Listener &listener) = 0;
    virtual void removeEditorPresentationStateListener(Listener &listener) = 0;
};
