#pragma once

#include "ui/analyzer/state/AnalyzerUiSnapshot.h"

class AnalyzerUiSnapshotSource {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void analyzerUiSnapshotChanged(const Ui::AnalyzerUiSnapshot &snapshot) = 0;
    };

    virtual ~AnalyzerUiSnapshotSource() = default;

    virtual Ui::AnalyzerUiSnapshot getAnalyzerUiSnapshot() const = 0;
    virtual void addAnalyzerUiSnapshotListener(Listener &listener) = 0;
    virtual void removeAnalyzerUiSnapshotListener(Listener &listener) = 0;
};
