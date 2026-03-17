# UI Architecture

This document describes the current editor and analyzer UI architecture from `SpectrumAnalyzerAudioProcessorEditor` down into the analyzer `view/`, `model/`, `popups/`, and helper layers.

## 1. Top-Level Editor Flow

```mermaid
flowchart TD
    Editor[SpectrumAnalyzerAudioProcessorEditor] --> Theme[Ui::Theme]
    Editor --> Panel[AnalyzerPanelComponent]
    Processor[SpectrumAnalyzerAudioProcessor] --> DataSource[AnalyzerDataSource]
    Processor --> UiState[AnalyzerUiStateSource]
    Processor --> Settings[AnalyzerSettingsActions]
    DataSource --> Panel
    UiState --> Panel
    Settings --> Panel
```

- The editor owns the theme and one `AnalyzerPanelComponent`.
- The processor is passed into the panel three times:
  - as `AnalyzerDataSource` for read-only UI state
  - as `AnalyzerUiStateSource` for discrete UI snapshots and change notifications
  - as `AnalyzerSettingsActions` for UI-triggered parameter changes

## 2. Editor Layout

```mermaid
flowchart TD
    Editor[SpectrumAnalyzerAudioProcessorEditor] --> Panel[AnalyzerPanelComponent]
    Panel --> Freeze[Freeze icon button]
    Panel --> Plot[AnalyzerComponent]
    Panel --> Rack[SignalRackComponent]
    Panel --> MeterRail[AnalyzerMeterControlsComponent]
```

Current layout in `AnalyzerPanelComponent`:

- top header:
  - snowflake freeze button on the right
- main body:
  - analyzer plot
- bottom strip:
  - left: signal rack
  - right: vertical meter toggle rail

## 3. UI Responsibilities By Layer

```mermaid
flowchart LR
    DataSource[AnalyzerDataSource] --> Plot[AnalyzerComponent]
    UiState[AnalyzerUiStateSource] --> Freeze[Freeze icon button]
    UiState --> Rack[SignalRackComponent]
    UiState --> MeterRail[AnalyzerMeterControlsComponent]

    Settings[AnalyzerSettingsActions] --> Rack
    Settings --> MeterRail
    Settings --> Freeze[Freeze icon button]
```

- `AnalyzerDataSource`
  - read-only analyzer render data
- `AnalyzerUiStateSource`
  - discrete UI snapshot for freeze, meter toggles, slot state, slot order, and sidechain availability
- `AnalyzerSettingsActions`
  - write path for user actions
  - semantic slot operations such as apply, remove, and add

This split keeps UI reads and UI writes explicit.

Current analyzer folder split:

- `src/ui/analyzer/view/`
  - JUCE components
- `src/ui/analyzer/model/`
  - derived UI state, policy, and refresh logic
- `src/ui/analyzer/popups/`
  - custom callout content
- `src/ui/analyzer/helpers/`
  - low-level geometry, smoothing, hover, and formatting helpers

## 4. AnalyzerPanelComponent

`AnalyzerPanelComponent` is the UI coordinator.

It owns:

- `AnalyzerComponent`
- `SignalRackComponent`
- `AnalyzerMeterControlsComponent`
- `freezeButton`

Responsibilities:

- lay out the analyzer panel
- route global freeze button clicks into `AnalyzerSettingsActions`
- subscribe to `AnalyzerUiStateSource` and keep the freeze button appearance in sync

It does not do analyzer rendering itself.

## 5. AnalyzerComponent Rendering Flow

```mermaid
flowchart TD
    Timer[UI poll timer\n~30 Hz default] --> Pull[pull latest UI snapshot]
    Pull --> Frozen{Frozen?}
    Pull --> Activity{recent signal or display decay?}
    Frozen -->|no| Meter[AnalyzerMeter.tick]
    Frozen -->|yes| Keep[keep previous renderData]
    Activity -->|no| Idle[slow idle polling]
    Activity -->|yes| Meter
    Pull --> RawFrame[latest completed DSP analysis frame]
    RawFrame --> Meter
    Meter --> LiveRender[live RenderData]
    LiveRender --> Compose[compose slot-frozen display traces]
    Keep --> RenderData
    Compose --> RenderData[display RenderData]
    RenderData --> Static[update static layout if bounds/scale changed]
    RenderData --> Dynamic[update dynamic trace bars]
    Hover[mouse move/drag/exit] --> HoverModel[update hover only]
    Static --> Paint[paint]
    Dynamic --> Paint
    HoverModel --> Paint
```

`AnalyzerComponent` owns:

- latest `bandInfo`
- latest `rawTraces`
- `AnalyzerMeter`
- `RenderData`
- `AnalyzerViewModel`
- `AnalyzerViewState`
- `AnalyzerRefreshModel`
- `Ui::AnalyzerConstants`
- hover position

Responsibilities:

- poll processor-backed analyzer data
- read the latest completed DSP analysis frame from the triple buffer
- use `AnalyzerRefreshModel` for UI snapshot refresh, freeze-edge handling, and idle-polling decisions
- convert raw traces into display-rate metered values
- compose per-slot frozen traces from cached last-painted `RenderTrace` data
- rebuild cached static layout only when geometry or scale inputs change
- rebuild dynamic bar geometry from the latest render data
- update hover state independently from the static and dynamic analyzer layers
- paint the analyzer plot, bars, grid, and hover tooltip

Important behavior:

- if frozen:
  - `renderData` is not advanced
  - UI presentation still updates from current slot/meter settings
- if an individual slot is frozen while global freeze is off:
  - the analyzer keeps metering live traces
  - that slot reuses its cached rendered trace until it is unfrozen
- the UI poll rate is only a display concern; DSP analysis frames are produced independently on the audio thread
- visible traces are rebuilt from current signal slot UI state on every refresh
- the analyzer background, frame, grid, and fixed labels are cached into a static image layer and only regenerated when needed
- hover movement repaints only the old/new tooltip and hovered band region instead of forcing a full analyzer redraw
- when the engine reports no recent signal, the analyzer keeps polling only long enough for the UI meter to decay to floor, then switches to a slower idle cadence until signal returns
- UI-facing analyzer constants such as visible frequency defaults, poll cadence, and frequency scale labels live in `src/ui/analyzer/AnalyzerUiConstants.h`, not in the DSP constants header

## 6. Analyzer Data Flow

```mermaid
flowchart LR
    Raw[vector RawTrace] --> Meter[AnalyzerMeter]
    Bands[vector BandInfo] --> Meter
    Meter --> Render[RenderData]
    Render --> ViewModel[AnalyzerViewModel]
    Slots[Ui::SignalSlotState array] --> ViewModel
    MeterSettings[MeterSettings] --> ViewModel
    GridSettings[grid min/max/step] --> ViewModel
    Hover[mouse position] --> ViewModel
    ViewModel --> TraceVisuals[AnalyzerTraceVisual list]
    TraceVisuals --> Paint[bar drawing]
```

Stages:

1. DSP publishes raw per-band measurements as `RawTrace` only when a fixed-size internal analysis frame completes
2. `AnalyzerMeter` converts those into:
   - RMS dB
   - peak dB
   - hold dB
3. `AnalyzerViewModel` converts render data into:
   - plot bounds
   - grid markers
   - frequency markers
   - per-trace bar rectangles
   - hover tooltip data

## 7. Analyzer Helper Layer

```mermaid
flowchart LR
    Meter[AnalyzerMeter] --> RenderData
    ViewModel[AnalyzerViewModel] --> Geometry[AnalyzerGeometry]
    ViewModel --> HoverModel[AnalyzerHoverModel]
    ViewModel --> Formatter[FrequencyFormatter]
    ViewModel --> Theory[MusicTheory]
```

Helper roles:

- `AnalyzerRefreshModel`
  - snapshot refresh, freeze-edge handling, and polling decisions
- `AnalyzerMeter`
  - UI-rate RMS averaging plus display decay/hold logic
  - peak hold refreshes immediately on a new maximum and also refreshes its timer for near-matching peaks within a small dB tolerance
- `AnalyzerViewModel`
  - split builder for static layout, dynamic trace visuals, and hover state
- `AnalyzerGeometry`
  - coordinate transforms and band hit-testing
- `AnalyzerHoverModel`
  - tooltip content and bounds
- `FrequencyFormatter`
  - scale/tooltip frequency labels
- `MusicTheory`
  - note labels for hovered frequencies

## 8. Signal Rack Architecture

```mermaid
flowchart TD
    Rack[SignalRackComponent] --> Slot0[SignalSlotComponent]
    Rack --> Slot1[SignalSlotComponent]
    Rack --> Slot2[SignalSlotComponent]
    Rack --> Slot3[SignalSlotComponent]
    Rack --> Add[+ add button]
```

`SignalRackComponent` owns:

- up to `4` `SignalSlotComponent`s
- one trailing `+` add button
- `SignalRackLayoutEngine`
- `SignalRackDragSession`

Responsibilities:

- subscribe to `AnalyzerUiStateSource`
- map enabled slots onto visible slot components
- pass down:
  - slot config/presentation
  - used colors
  - used signal configurations
  - sidechain availability
- wire slot actions into `AnalyzerSettingsActions`
- expose per-slot visibility and freeze actions directly on each slot cell
- add a new signal into the first free slot
- keep direct slot interactions visually immediate with local optimistic updates, then reconcile back to processor state
- delegate slot default-selection policy to `src/ui/analyzer/model/SignalRackModel.h`

Reordering responsibilities are split out:

- `SignalRackLayoutEngine`
  - deterministic bounds for visible slot cells
  - active rack span used by drag/reorder
- `SignalRackDragSession`
  - drag session state
  - floating dragged-cell bounds
  - preview-order calculation and final commit order
- `SignalSlotOrderModel`
  - ordered-slot and trace-order helper logic
- `SignalRackModel`
  - pure helpers for choosing a free slot, default signal option, default colour, and append-to-end order

Current behavior:

- disabled slots are not shown
- hidden slots stay in the rack but render as `Off`
- add button is visible only while active slot count is below `Shared::maxSignalSlots`
- drag-reordering changes only UI order, not DSP slot identity
- the dragged slot is painted as a floating overlay while the real child component stays anchored
- the `+` add cell is square and lives outside the reorderable active-slot span

## 9. Signal Slot Component

```mermaid
flowchart LR
    Slot[SignalSlotComponent] --> Swatch[color swatch]
    Slot --> Label[mode label + source hint]
    Slot --> Toggle[visibility toggle]
    Slot --> Remove[remove button]
```

Each `SignalSlotComponent` shows:

- a color swatch
- main label: `Mid`, `Side`, or `Stereo`
- secondary hint: `Main` or `Sidechain`
- visibility button: `On` / `Off`
- remove button: `x`

Supported interactions:

- click label:
  - toggle signal picker callout
- click swatch:
  - toggle preset color picker callout
- drag vertically on swatch:
  - adjust opacity
- double click swatch:
  - reset opacity to default
- click visibility:
  - toggle visible/hidden
- click remove:
  - disable slot

Responsibility split:

- `SignalSlotComponent`
  - slot painting
  - hit-testing
  - opacity drag
  - reorder drag gesture forwarding
  - callout lifecycle
- `SignalSelectionPopupContent`
  - signal-option callout rows and section layout
- `SignalColourPopupContent`
  - colour grid callout layout
- `SignalSlotOptions`
  - single source of truth for supported source/mode combinations and labels

Picker rules:

- duplicate source/mode combinations are disabled
- duplicate preset colors are disabled
- slot picker options are driven from `src/ui/analyzer/model/SignalSlotOptions.h`
- when sidechain is unavailable:
  - signal picker shows one section with `Mid / Side / Stereo`
- when sidechain is available:
  - signal picker shows `Main` and `Sidechain` sections inside one callout
- both signal and color pickers are custom `CallOutBox` content, not `PopupMenu`
- clicking the same opener again dismisses the currently open callout

## 10. Meter Controls Component

```mermaid
flowchart TD
    MeterRail[AnalyzerMeterControlsComponent] --> Peak[Peak button]
    MeterRail --> RMS[RMS button]
    MeterRail --> Hold[Hold button]
```

Responsibilities:

- subscribe to `AnalyzerUiStateSource`
- keep the three buttons visually synced
- send toggles through `AnalyzerSettingsActions`

Current styling:

- disabled:
  - neutral `controlSurface`
  - standard control text
- enabled:
  - brighter neutral `controlSurfaceHover`
  - same control-text family
- the meter buttons intentionally use the same neutral selected-surface language as the signal picker rows

## 11. UI State Types

```mermaid
flowchart LR
    SignalSlotState[Ui::SignalSlotState] --> Config[SignalSlotConfiguration]
    SignalSlotState --> Visible[visible]
    SignalSlotState --> Colour[colourIndex]
    SignalSlotState --> Opacity[opacity]

    SlotOrder[slotOrder array] --> RackOrder[rack display order]
    SlotOrder --> PaintOrder[overlay stacking order]
    SlotOrder --> HoverOrder[primary hover trace order]

    ViewState[AnalyzerViewState] --> EnabledTraces[enabledTraces]
    ViewState --> FrequencyRange[visible frequency range]
```

Important UI-facing state types:

- `Ui::SignalSlotState`
  - one slot’s render and interaction state
- `Ui::AnalyzerUiState`
  - immutable editor snapshot for discrete controls and rack state
- `std::array<size_t, Shared::maxSignalSlots>`
  - persistent UI slot order
- `AnalyzerViewState`
  - currently visible traces and optional frequency-range override
- `Analyzer::RenderData`
  - render-ready meter output

The processor owns `plugin/state/SignalSlotOrderState`, which persists the slot order and exposes it through `AnalyzerDataSource`.
The engine does not use this order.

Important order/detail notes:

- rack order is left-to-right by `slotOrder`
- hover priority follows that same order
- paint order follows rack order, so the rightmost/later cell is drawn last and appears on top
- `AnalyzerUiState` is published from the processor on the message thread via `AnalyzerUiStateSource`

## 12. Theme Layer

`Ui::Theme` centralizes colors for:

- editor background
- analyzer background and plot surface
- controls
- grid and axis
- tooltip
- accent buttons

Signal overlays do not come from the main theme accent.
They come from the preset signal palette:

- `Ui::getSignalPresetColour(int)`
- `Ui::getSignalPresetName(int)`
- both read from the shared preset catalog used by UI and parameter metadata

Each visible trace uses:

- preset color
- slot opacity

## 13. Current Limitations

- Hover currently uses the leftmost visible trace as its data source.
- `AnalyzerComponent` still uses a timer, intentionally, because analyzer render data is continuous and display-rate driven.
- Slot reordering is UI-only and currently stored through plugin state serialization, not APVTS parameters.

## Guiding Rule

The UI is split into:

- read-only data access through `AnalyzerDataSource`
- write actions through `AnalyzerSettingsActions`
- display-rate analyzer rendering in `AnalyzerComponent`
- signal configuration UI in `SignalRackComponent` and `SignalSlotComponent`
- slot-option and default-selection policy in `src/ui/analyzer/model/`
- refresh/state-transition logic in `src/ui/analyzer/model/AnalyzerRefreshModel.*`
- custom callout content in `src/ui/analyzer/popups/`

The processor remains the bridge between APVTS-backed state and the editor.
Inside the plugin layer, typed parameter reads and writes now go through `plugin/parameters/ParameterAccess`, backed by the single parameter-definition source in `plugin/parameters/ParameterSchema`.
The analyzer helpers keep rendering logic out of the top-level components.
