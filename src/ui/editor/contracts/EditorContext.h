#pragma once

class AnalyzerRawTraceSource;
class AnalyzerUiSnapshotSource;
class AnalyzerSettingsActions;
class PresetUiSnapshotSource;
class PresetActions;
class EditorPresentationStateSource;
class EditorPresentationActions;

namespace Ui {
    /** Plugin-implemented contracts the editor consumes; references must outlive the editor. */
    struct EditorContext {
        AnalyzerRawTraceSource &rawTraceSource;
        AnalyzerUiSnapshotSource &analyzerUiSnapshotSource;
        PresetUiSnapshotSource &presetUiSnapshotSource;
        AnalyzerSettingsActions &analyzerSettingsActions;
        PresetActions &presetActions;
        EditorPresentationStateSource &editorPresentationStateSource;
        EditorPresentationActions &editorPresentationActions;
    };
}
