# Architecture Rules

## Layers

- `src/dsp/`
  - audio-domain code only
  - no UI concerns
  - no APVTS knowledge

- `src/plugin/`
  - composition root only
  - owns APVTS, serialization, and wiring between DSP and UI
  - publishes render data and immutable UI snapshots

- `src/ui/`
  - presentation and interaction only
  - no APVTS access
  - no DSP processing logic

## Analyzer UI Contracts

- High-frequency analyzer render data must flow through `AnalyzerRenderSource`.
- Low-frequency analyzer UI/control state must flow through `AnalyzerUiSnapshotSource`.
- UI writes must flow through `AnalyzerSettingsActions`.
- Render transport and UI snapshot transport must remain separate.
- The triple buffer is only for analyzer render data, never for UI snapshot payloads.

## Snapshot Rules

- `Ui::AnalyzerUiSnapshot` is the only analyzer UI snapshot type.
- Do not introduce parallel snapshot structs for the same state.
- Views may cache the latest published snapshot for rendering, but may not keep competing writable business-state copies.
- If state can be derived from the snapshot, do not store it as separate mutable state.
- Cross-trace analyzer overlays derived from what is currently drawn must live in analyzer model/view-model logic, not in the snapshot.

## UI Structure

- `view/`
  - JUCE `Component` classes only
  - no APVTS access
  - no duplicated business logic

- `model/`
  - snapshot types
  - selectors
  - option metadata
  - refresh/state-transition logic
  - presentation policy

- `popups/`
  - reusable popup content components only

- `helpers/`
  - low-level math, geometry, formatting, and render utilities only

## Source Of Truth Rules

- Repeated labels, options, and mappings must be defined once in a shared metadata table.
- Popup, tooltip, axis, button, slot, and divider geometry/styling must come from owned config/theme tokens.
- Analyzer-specific presentation config must live under analyzer-owned modules or analyzer-owned theme sections, not generic shared UI buckets.
- Shared UI modules must not accumulate analyzer-only policy.
- Do not model analyzer overlays such as global hold as fake traces just to reuse slot/trace plumbing.

## Action Rules

- Views may call `AnalyzerSettingsActions`, but may not know parameter ids.
- Command composition must live in one place only.
- If adding a new UI action requires duplicating behavior between a default interface and the processor implementation, the design needs adjustment.

## General Rules

- A file should have one reason to change.
- Hot paths stay concrete: avoid unnecessary virtual indirection in DSP and analyzer rendering.
- Keep long-lived DSP state separate from per-block scratch so reset semantics stay obvious and hot loops stay easy to reason about.
- If adding a new signal mode touches many unrelated files, the design needs adjustment.
