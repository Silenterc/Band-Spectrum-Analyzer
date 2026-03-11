#pragma once

#include "AnalyzerUiState.h"

class AnalyzerUiStateSource {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void analyzerUiStateChanged(const Ui::AnalyzerUiState &state) = 0;
    };

    virtual ~AnalyzerUiStateSource() = default;

    virtual Ui::AnalyzerUiState getAnalyzerUiState() const = 0;
    virtual void addAnalyzerUiStateListener(Listener &listener) = 0;
    virtual void removeAnalyzerUiStateListener(Listener &listener) = 0;
};
