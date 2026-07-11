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
- `PluginUiBridge` is the only implementer of UI-facing contracts; `SpectrumAnalyzerAudioProcessor` must not implement them directly
- may depend on UI contract/state headers to publish UI-facing snapshots
- bridges parameter state into:
  - engine-facing DSP state
  - UI-facing snapshot state
  - worker-facing semantic display control state
- maps plugin-domain persistence/session data into UI-domain snapshots/results
- assembles `Ui::EditorContext` in `createEditor()`; the editor consumes contracts only through it

### `src/ui/`

- message-thread presentation and interaction only
- no DSP processing logic
- no worker-owned time evolution
- no APVTS access
- no includes from `src/plugin/`
- may consume immutable display frames and immutable UI snapshots
- owns UI-facing state/contract types used by UI features

## Contract Rules

Analyzer channels and UI feature channels must stay separate.

### 1. DSP-to-display raw trace channel

- contract: `AnalyzerRawTraceSource`, owned by `src/dsp/core/` and implemented by `Analyzer::Engine`
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

- contracts:
  - `AnalyzerSettingsActions`
  - `EditorPresentationActions`
- views dispatch intents only
- views must not know parameter ids or APVTS details

### 4. Preset UI channels

- contracts:
  - `PresetUiSnapshotSource`
  - `PresetActions`
- payloads:
  - `Ui::Presets::PresetUiSnapshot`
  - `Ui::Presets::PresetActionResult`
- plugin preset documents, XML, file stores, and APVTS snapshots remain plugin-domain types
- UI must not include `plugin/presets/*`
- UI contracts live with their feature, not in a central `src/ui/contracts/` folder

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
- feature snapshots should live with their UI feature state, such as `Ui::Presets::PresetUiSnapshot`
- plugin code may map plugin-domain state into UI snapshots, but UI code must not consume plugin-domain state directly

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

### `src/ui/analyzer/plot/state/`

- plot UI constants, view state, and immutable layout result structs
- no JUCE `Component` classes
- do not add `src/ui/analyzer/plot/data/`

### `src/ui/analyzer/plot/view/`

- JUCE `Component` classes only
- consume immutable frames and snapshots
- dispatch actions through contracts

### `src/ui/analyzer/controls/view/`

- analyzer control-strip components only
- no direct `.h` or `.cpp` files under `src/ui/analyzer/controls/`

### `src/ui/editor/view/`

- editor root and background JUCE components
- no direct `.h` or `.cpp` files under `src/ui/editor/`

### UI feature folders

Each UI feature should use the same responsibility split:

- `state/`: immutable UI snapshots, descriptors, view state, and action results
- `contracts/`: read-only snapshot sources and write-only action interfaces
- `model/` or `logic/`: pure selectors, layout math, formatting, and availability rules
- `interaction/`: transient state machines such as drag sessions or keyboard selection
- `view/`: JUCE components only
- `popups/`: popup content only

Small features may omit empty folders, but they must not merge plugin persistence, component ownership, and interaction policy into the same file.

Legacy central UI folders are not allowed for new or moved feature code:

- do not add `src/ui/contracts/`
- do not add `src/ui/state/`
- keep analyzer contracts under `src/ui/analyzer/contracts/`
- keep preset contracts under `src/ui/presets/contracts/`
- keep editor contracts under `src/ui/editor/contracts/`

### `src/ui/widgets/`

- shared UI primitives and popup/callout helpers
- no analyzer-specific or preset-persistence policy
- callout lifetime, look-and-feel cleanup, and common launch/dismiss behavior should be centralized here

### `src/shared/`

- cross-layer value types, defaults, and small option catalogs shared by plugin and UI
- must not include `src/ui/*`
- if plugin and UI both need a small value enum, put that value type here and let UI feature state compose it
- shared option catalogs may pair such enums with stable host/UI labels when both layers must use the same mapping

## Source Of Truth Rules

- repeated labels, options, and mappings must be defined once
- appearance, geometry, and repeated interaction metrics must come from theme/config ownership
- analyzer-specific policy must not leak into generic shared UI modules
- popup content must not dismiss `juce::CallOutBox` directly
- popup owners/presenters own launch, dismissal, focus return, and look-and-feel cleanup
- worker semantic state must not be duplicated in UI view code
- hold overlays must remain their own semantic output, not be modeled as fake slot traces
- plugin-domain preset documents and serialized state must not leak into UI contracts or views

## Optimistic Local Echo Rules

Direct controls may update their own visual state immediately after user input, then reconcile with the next authoritative snapshot.

Rules:

- local echo is component-local and temporary
- the next snapshot from the source wins
- local echo must not be published as a separate mutable feature state model
- use it only for direct interaction feedback, such as slot toggles, colour/mode selection, and successful popup row removal

## Paired Settings Constraint Rules

The settings page may tighten the interactive range of paired controls to make direct manipulation predictable. Current examples are the minimum separation between grid bounds and the minimum ratio between visible frequency bounds.

Rules:

- interactive limits are UI affordances, not global APVTS invariants
- `PluginUiBridge` may constrain settings-page intents before writing a parameter
- the underlying parameters remain independently addressable by host automation and restored plugin state
- snapshots publish the stored parameter values; they must not silently rewrite host-authored values
- degenerate host-authored combinations are accepted as independent parameter state rather than repaired by changing another parameter

## Change Rules

If a change crosses these boundaries, stop and check the design.

Examples of suspicious changes:

- adding slot order to `AnalyzerDisplayControlState`
- adding hover state to `src/display/`
- adding APVTS knowledge to `src/ui/`
- implementing a UI contract on `SpectrumAnalyzerAudioProcessor` instead of `PluginUiBridge`
- including `src/plugin/*` from `src/ui/`
- including `src/ui/*` from `src/shared/`
- exposing plugin preset documents through `PresetUiSnapshotSource`
- calling `findParentComponentOfClass<juce::CallOutBox>()` from popup content
- adding JUCE geometry types to worker data structs
- adding colour/opacity handling to the display worker
- adding hold logic back into `AnalyzerPlotComponent`

## General Rules

- a file should have one reason to change
- hot paths stay concrete and easy to profile
- long-lived state stays separate from transient scratch
- thread ownership should be obvious from the file path
- if a new analyzer feature cannot be placed cleanly into `dsp`, `display`, `plugin`, or `ui`, the design is probably wrong
