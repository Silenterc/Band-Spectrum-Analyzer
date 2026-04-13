# Architecture Rules

This project now has four architectural layers:

- `src/dsp/`
- `src/display/`
- `src/plugin/`
- `src/ui/`

The intent is simple:

- `dsp` owns audio-thread analyzer computation
- `display` owns worker-thread display-state computation
- `plugin` owns APVTS, persistence, and cross-layer wiring
- `ui` owns message-thread presentation and interaction

## Layer Rules

### `src/dsp/`

- audio-thread code only
- no JUCE `Component` code
- no APVTS access
- no UI theme, colour, order, opacity, hover, or layout policy
- may publish raw analyzer measurements through lock-free transport

### `src/display/`

- worker-thread analyzer display pipeline only
- owns metering, per-slot freeze latching, contribution reduction, and hold evolution
- may consume DSP-published raw traces
- may publish immutable display frames for the UI
- must not know JUCE component geometry, hover, slot order, or paint batching

### `src/plugin/`

- composition root only
- owns APVTS, serialization, and listener wiring
- implements contracts exposed to `display` and `ui`
- bridges parameter state into:
  - engine-facing DSP state
  - UI-facing snapshot state
  - worker-facing semantic display control state

### `src/ui/`

- message-thread presentation and interaction only
- no DSP processing logic
- no worker-owned time evolution
- no APVTS access
- may consume immutable display frames and immutable UI snapshots

## Contract Rules

There are now three analyzer read/write channels and they must stay separate.

### 1. DSP-to-display raw trace channel

- contract: `AnalyzerRawTraceSource`
- transport payload:
  - `bandInfo`
  - `AnalyzerPublishedTracesView`
  - recent-signal activity
- this is the only high-frequency analyzer data path out of DSP
- this path is raw measurement transport, not UI state transport

### 2. Plugin-to-UI snapshot channel

- contract: `AnalyzerUiSnapshotSource`
- payload: `Ui::AnalyzerUiSnapshot`
- this is the only immutable low-frequency analyzer UI snapshot type
- snapshots must not carry trace payloads

### 3. UI-to-plugin action channel

- contract: `AnalyzerSettingsActions`
- views dispatch intents only
- views must not know parameter ids or APVTS details

## Ownership Rules

### Worker-owned analyzer state

The following kinds of state belong in `src/display/`, not `src/ui/`:

- display-rate meter decay
- per-slot frozen-frame capture/latching
- contribution reduction for hold semantics
- global hold timing and decay
- active vs idle display polling cadence
- cross-thread display-frame publication

### UI-owned analyzer state

The following kinds of state belong in `src/ui/`, not `src/display/`:

- slot order
- colour and opacity styling
- hover position and tooltip content
- visible frequency range / zoom
- geometry, static layout, and paint batches
- repaint decisions

### DSP-owned analyzer state

The following kinds of state belong in `src/dsp/`, not `src/display/` or `src/ui/`:

- filter-bank state
- accumulated band measurements
- raw trace publication
- sample-rate dependent band layout generation
- recent-input activity detection

## Triple Buffer Rules

There are two distinct lock-free publication boundaries now:

- `dsp -> display`
  - `TripleBuffer<vector<RawTrace>>`
- `display -> ui`
  - `TripleBuffer<AnalyzerDisplayFrame>`

Rules:

- do not use UI snapshots as a substitute for either triple buffer
- do not publish APVTS-backed UI state through the triple buffer
- do not publish paint-ready JUCE geometry through the DSP triple buffer
- keep both triple-buffer paths single-writer / single-reader

## Snapshot Rules

- `Ui::AnalyzerUiSnapshot` is still the only analyzer UI snapshot type
- do not introduce a parallel mutable UI state model for the same data
- if state can be derived from the snapshot and the latest display frame, do not store a competing writable copy
- cross-thread semantic state for the worker must use `AnalyzerDisplayControlState`, not `AnalyzerUiSnapshot`

`AnalyzerDisplayControlState` exists because the worker needs a reduced semantic view of UI state:

- global freeze
- meter settings
- per-slot frozen flags
- per-slot contribution flags
- floor dB

It must not grow presentation-only fields such as:

- slot order
- colour
- opacity
- hover state
- bounds

## Folder Rules

### `src/display/analyzer/contracts/`

- contracts between DSP/plugin and the worker
- non-owning read views only
- no long-lived worker logic

### `src/display/analyzer/data/`

- worker-domain structs only
- immutable or plain data carriers
- no JUCE component geometry
- no APVTS references

### `src/display/analyzer/logic/`

- worker-owned state machines and semantic transforms
- no paint batching
- no slot ordering
- no colour/opacity policy

### `src/display/analyzer/thread/`

- thread lifecycle
- wake/sleep policy
- publication buffers
- cross-thread handoff mechanics

### `src/ui/analyzer/plot/logic/`

- message-thread layout, geometry, hover, formatting, and paint batching only
- no meter decay
- no hold evolution
- no refresh cadence state machine

### `src/ui/analyzer/plot/view/`

- JUCE `Component` classes only
- consume immutable frames and snapshots
- dispatch actions through contracts

## Source Of Truth Rules

- repeated labels, options, and mappings must be defined once
- appearance, geometry, and repeated interaction metrics must come from theme/config ownership
- analyzer-specific policy must not leak into generic shared UI modules
- worker semantic state must not be duplicated in UI view code
- hold overlays must remain their own semantic output, not be modeled as fake slot traces

## Change Rules

If a change crosses these boundaries, stop and check the design.

Examples of suspicious changes:

- adding slot order to `AnalyzerDisplayControlState`
- adding hover state to `src/display/`
- adding APVTS knowledge to `src/ui/`
- adding JUCE geometry types to worker data structs
- adding colour/opacity handling to the display worker
- adding hold logic back into `AnalyzerPlotComponent`

## General Rules

- a file should have one reason to change
- hot paths stay concrete and easy to profile
- long-lived state stays separate from transient scratch
- thread ownership should be obvious from the file path
- if a new analyzer feature cannot be placed cleanly into `dsp`, `display`, `plugin`, or `ui`, the design is probably wrong
