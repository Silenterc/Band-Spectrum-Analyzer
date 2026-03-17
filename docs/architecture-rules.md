# Architecture Rules

## Layers

- `src/dsp/`
  - audio-domain code only
  - no UI concerns
  - no APVTS knowledge

- `src/plugin/`
  - parameter and state bridge
  - owns APVTS, serialization, and wiring between DSP and UI
  - acts as the composition root

- `src/ui/`
  - presentation and interaction only
  - no APVTS access
  - no DSP processing logic

## UI Structure

- `view/`
  - JUCE `Component` classes only

- `model/`
  - pure UI policy
  - derived UI state
  - option tables and refresh/state-transition logic

- `popups/`
  - reusable custom callout content

- `helpers/`
  - low-level math, geometry, formatting, and render utilities

## Rules

- A file should have one reason to change.
- Views may call `AnalyzerSettingsActions`, but may not know parameter ids.
- UI models may not access APVTS directly.
- DSP code may not include UI headers.
- Repeated labels, options, and mappings must be defined once in a shared table.
- Hot paths stay concrete: avoid unnecessary virtual indirection in DSP and analyzer rendering.
- Keep long-lived DSP state separate from per-block scratch so reset semantics stay obvious and hot loops stay easy to reason about.
- If adding a new signal mode touches many unrelated files, the design needs adjustment.
