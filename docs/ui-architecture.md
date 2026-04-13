# UI Architecture

This document describes the current UI architecture after the analyzer split into:

- a DSP raw-trace channel
- a worker-thread display channel
- a low-frequency immutable UI snapshot channel
- a write-only action channel

## Top-Level Flow

```mermaid
flowchart TD
    Editor[SpectrumAnalyzerAudioProcessorEditor] --> Theme[Ui::Theme]
    Editor --> Layout[MainLayoutComponent]

    Processor[SpectrumAnalyzerAudioProcessor] --> RawSource[AnalyzerRawTraceSource]
    Processor --> Snapshot[AnalyzerUiSnapshotSource]
    Processor --> Actions[AnalyzerSettingsActions]

    RawSource --> AnalyzerSection[AnalyzerSectionComponent]
    Snapshot --> AnalyzerSection
    Snapshot --> Rack[SignalRackComponent]
    Snapshot --> Strip[AnalyzerMeterControlsComponent]
    Actions --> Rack
    Actions --> Strip
```

The editor still owns:

- theme
- top-level layout

The processor still owns:

- contracts
- snapshot publication
- action handling

The analyzer view is now split into:

- `AnalyzerSectionComponent`
  - static shell, labels, tooltip, and layout ownership
- `AnalyzerPlotComponent`
  - exact plot child, opaque hot surface, and worker-frame consumption

Only the plot child repaints at the analyzer frame rate.

## Read / Write Channels

### 1. Raw Trace Channel

`AnalyzerRawTraceSource` owns:

- `getBandInfo()`
- `readPublishedTraces()`
- `hasRecentSignal()`

Rules:

- this is not a UI snapshot channel
- this is not paint-ready data
- the message thread should not consume raw traces directly
- this contract primarily feeds the display worker

### 2. Display Frame Channel

This channel is internal to the analyzer section:

- producer: `AnalyzerDisplayWorker`
- consumer: `AnalyzerPlotComponent`

Transport:

- `AnalyzerDisplayFrameBuffer`
- `TripleBuffer<AnalyzerDisplayFrame>`

Rules:

- published frames are immutable
- frames are semantic display state, not JUCE geometry
- frames are keyed by slot index, not visual order

### 3. UI Snapshot Channel

`AnalyzerUiSnapshotSource` owns:

- `getAnalyzerUiSnapshot()`
- snapshot listener registration

`Ui::AnalyzerUiSnapshot` remains the only low-frequency analyzer UI snapshot type.

It contains:

- `signalSlots`
- `slotOrder`
- `meterSettings`
- `frozen`
- `sidechainAvailable`
- `gridMinDb`
- `gridMaxDb`
- `gridStepDb`

### 4. Action Channel

`AnalyzerSettingsActions` is still the only UI write path.

Rules:

- views dispatch intents only
- views must not know parameter ids
- views must not talk to APVTS directly

## Folder Responsibilities

### `src/ui/`

- shared UI primitives
- theme tokens
- popup chrome
- icons and assets
- no analyzer-specific worker logic

### `src/ui/analyzer/controls/`

- analyzer control strip components only
- snapshot-driven
- action-dispatching

### `src/ui/analyzer/layout/`

- top-level analyzer area composition only

### `src/ui/analyzer/plot/view/`

- JUCE `Component` classes only
- analyzer section container
- analyzer plot component
- tooltip helper

### `src/ui/analyzer/plot/logic/`

Now message-thread-only and presentation-only:

- `AnalyzerViewModel`
  - canonical `AnalyzerSectionLayout`
  - section-space labels and plot bounds
  - plot-local band and grid geometry
  - hover lookup from plot-local cursor input
- `AnalyzerRenderBatchBuilder`
  - direct paint-batch generation from immutable display frames
- `AnalyzerGeometry`
  - domain-to-screen mapping
- formatting and hover helpers

This folder no longer owns:

- meter decay
- display composition
- global hold evolution
- refresh cadence

Those moved to `src/display/analyzer/`.

### `src/ui/analyzer/rack/`

- rack interaction
- rack layout
- rack popups
- slot controls and slot visuals

### `src/ui/analyzer/state/`

- immutable analyzer UI snapshot types only

## Analyzer Plot Flow

```mermaid
flowchart TD
    SnapshotEvents[AnalyzerUiSnapshotSource listener] --> Snapshot[Ui::AnalyzerUiSnapshot]
    Worker[AnalyzerDisplayWorker] --> Frame[AnalyzerDisplayFrame]
    Snapshot --> Section[AnalyzerSectionComponent]
    Frame --> Plot[AnalyzerPlotComponent]
    Section --> Layout[AnalyzerViewModel canonical layout]
    Layout --> Plot
    Layout --> Tooltip[AnalyzerHoverTooltipRenderer]
    Snapshot --> Plot
    Plot --> Batches[AnalyzerRenderBatchBuilder]
```

Important behavior:

- `AnalyzerSectionComponent` owns layout, static shell repaint policy, and tooltip state
- `AnalyzerPlotComponent` consumes immutable display frames from the worker
- snapshot changes are split into:
  - layout-affecting changes handled by the section shell
  - plot-only presentation changes handled by the plot child
- only the opaque plot child redraws bars, hold, and hover highlight every frame

## `AnalyzerSectionComponent` Responsibilities

`AnalyzerSectionComponent` now owns:

- canonical analyzer layout
- section background cache
- axis labels and outer plot chrome
- plot child bounds
- hover tooltip state
- plot-local hover to tooltip translation

It does not own:

- analyzer worker state
- bar/hold batch generation
- steady-state analyzer repaint cadence

## `AnalyzerPlotComponent` Responsibilities

`AnalyzerPlotComponent` now owns:

- `AnalyzerDisplayWorker`
- latest consumed display frame pointer
- opaque plot-base cache
- dynamic batch rebuilds
- plot-only repaint decisions
- in-plot hover highlight

It does not own:

- `AnalyzerMeter`
- display composition
- hold evolution
- refresh cadence state machine
- copied raw trace vectors

## Presentation vs Semantic State

This split is the most important UI rule in the current design.

### Semantic state

Semantic state affects time-evolving display behavior and is submitted to the worker through `AnalyzerDisplayControlState`.

Examples:

- global freeze
- per-slot freeze
- hold visibility/settings
- contribution mask derived from slot visibility
- floor dB

### Presentation state

Presentation state stays on the message thread and must not wake the worker.

Examples:

- slot order
- colour
- opacity
- hover
- zoom / visible frequency range

Practical consequence:

- opacity drag is UI-only
- reorder drag is UI-only
- freeze and hold still go through the worker

## Static Layout vs Dynamic Batching

The plot code is now split into two presentation stages.

### Static layout

Owned by `AnalyzerViewModel`.

Inputs:

- band layout
- section bounds
- analyzer display bounds
- grid min/max/step
- visible frequency range

Outputs:

- `AnalyzerSectionLayout`
- section-space axis labels
- section-space plot bounds and frame bounds
- plot-local visible bands
- plot-local grid and frequency marker positions

### Dynamic batching

Owned by `AnalyzerRenderBatchBuilder`.

Inputs:

- latest `AnalyzerDisplayFrame`
- visible bands
- slot order
- slot colour / opacity
- meter visibility
- plot bounds

Outputs:

- rectangle batches for bars
- rectangle batches for hold
- rectangle batches for hover highlight

This removes the old extra pass that built full intermediate trace/bar models every frame.

## Hover Split

Hover is now split across the shell and the plot child.

`AnalyzerPlotComponent` owns:

- hovered-band highlight batches
- plot-only dirty repaint bounds
- raw mouse tracking inside the exact plot rect

`AnalyzerHoverTooltipRenderer` remains a lightweight helper owned by `AnalyzerSectionComponent`.

It owns:

- tooltip chrome and glyphs

It consumes:

- hover info from `AnalyzerViewModel`

It does not own analyzer semantics.

## Theme Ownership

`Ui::Theme` remains the owner of visual tokens and repeated interaction metrics.

Important groups:

- `metrics.editor`
- `metrics.popup`
- `metrics.analyzerPlot`
- `metrics.tooltip`
- `metrics.slot`
- `metrics.sectionDivider`

Rule:

- if a literal affects appearance, geometry, or repeated interaction behavior, it belongs in theme/config, not in a view file

## Triple Buffer Boundary

The UI now consumes the second triple-buffer boundary, not the first one.

- DSP triple buffer
  - raw traces
  - reader: display worker
- display triple buffer
  - display frames
  - reader: message thread

UI snapshots remain separate from both.

## Current State

- analyzer plot is no longer timer-driven for semantic recomputation
- worker thread owns display-rate analyzer state
- section shell owns static layout and tooltip presentation
- opaque plot child owns hot-path plotting work
- slot order, colour, and opacity are UI-only concerns
- hold and freeze semantics are worker-owned concerns
- analyzer docs and code now align on the explicit `dsp / display / ui` split
