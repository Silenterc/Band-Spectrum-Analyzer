# Audio Processing Architecture

This document describes the current backend and analyzer data flow from `SpectrumAnalyzerAudioProcessor` down into the analyzer DSP engine and back up into the UI.

## 1. Audio Callback Flow

```mermaid
flowchart TD
    Host[Host audio block] --> Processor[SpectrumAnalyzerAudioProcessor]
    Processor --> Params[readCurrentEngineParameters]
    Params --> Engine[Analyzer::Engine]
    Processor -->|main buffer| Engine
    Processor -->|optional sidechain buffer| Engine
    Engine --> SourceBuilder[AnalysisSourceBuilder]
    Engine --> PlanBuilder[AnalysisPlanBuilder]
    PlanBuilder --> Processors[AnalysisGroupProcessor list]
    Processors --> Published[TripleBuffer vector RawTrace]
```

## 2. Processor Responsibilities

```mermaid
flowchart TD
    Processor[SpectrumAnalyzerAudioProcessor] --> APVTS[APVTS parameters]
    Processor --> Engine[Analyzer::Engine]
    Processor --> DataSource[AnalyzerDataSource]
    Processor --> Settings[AnalyzerSettingsActions]

    APVTS --> EngineParams[EngineParameterState]
    APVTS --> UiSlots[Ui::SignalSlotState array]
    APVTS --> Meter[MeterSettings]
    APVTS --> Freeze[freeze state]
```

- The processor reads only `EngineParameterState` on the audio thread.
- The UI reads slot presentation state, freeze state, grid settings, and meter settings through `AnalyzerDataSource`.
- The processor also exposes UI write actions through `AnalyzerSettingsActions`.
- APVTS state plus persistent UI-only slot order are serialized in `getStateInformation()` / `setStateInformation()`.

## 3. Engine Internals

```mermaid
flowchart TD
    Engine[Analyzer::Engine] --> Bands[shared band layout]
    Engine --> SourceBuilder[AnalysisSourceBuilder]
    Engine --> PlanBuilder[AnalysisPlanBuilder]
    Engine --> Processors[vector AnalysisGroupProcessor]
    Engine --> Published[TripleBuffer vector RawTrace]

    SourceBuilder --> MainViews[mainLeft / mainRight]
    SourceBuilder --> MainDerived[mainMid / mainSide]
    SourceBuilder --> SideViews[sidechainLeft / sidechainRight]
    SourceBuilder --> SideDerived[sidechainMid / sidechainSide]

    PlanBuilder --> Specs[one AnalysisGroupSpec per enabled slot]
    Specs --> Processors
```

## 4. Engine Internals As A Tree

```text
SpectrumAnalyzerAudioProcessor
└── Analyzer::Engine
    ├── shared_ptr<vector<BandInfo>> bandInfo
    ├── AnalysisSourceBuilder sourceBuilder
    │   ├── SourceSet
    │   │   ├── mainLeft / mainRight
    │   │   ├── mainMid / mainSide
    │   │   └── sidechainLeft / sidechainRight / sidechainMid / sidechainSide
    │   └── engine-owned derived buffers
    │       ├── mainMidBuffer / mainSideBuffer
    │       └── sidechainMidBuffer / sidechainSideBuffer
    ├── AnalysisPlanBuilder planBuilder
    ├── vector<AnalysisGroupProcessor> processors
    │   └── each AnalysisGroupProcessor owns:
    │       ├── AnalysisGroupSpec
    │       │   ├── SourceFamily
    │       │   ├── vector<AnalysisLaneSpec>
    │       │   └── vector<AnalysisOutputSpec>
    │       ├── vector<BandState>
    │       ├── vector<vector<BandMeasurements>> outputMeasurements
    │       └── reused process scratch
    ├── size_t publishedTraceCount
    └── TripleBuffer<vector<RawTrace>> traces
```

## 5. Slot-Based Signal Model

```mermaid
flowchart LR
    Slot[Signal slot] --> Config[SignalSlotConfiguration]
    Config --> Enabled[enabled]
    Config --> Source[main / sidechain]
    Config --> Mode[mid / side / stereo]

    Slot --> UiState[Ui::SignalSlotState]
    UiState --> Visible[visible]
    UiState --> Colour[colourIndex]
    UiState --> Opacity[opacity]
```

- The engine uses `SignalSlotConfiguration` only.
- The UI uses `Ui::SignalSlotState`, which wraps slot configuration plus presentation data.
- Up to `4` slots are supported through `Shared::maxSignalSlots`.
- Each enabled slot becomes one published trace identity: `TraceKind::slot1` through `TraceKind::slot4`.

## 6. Analysis Plan Model

```mermaid
flowchart LR
    EngineParams[EngineParameterState] --> PlanBuilder[AnalysisPlanBuilder]
    PlanBuilder --> GroupSpecs[vector AnalysisGroupSpec]
    GroupSpecs --> Group[one spec per enabled slot]
    Group --> SourceFamily
    Group --> Lanes[vector AnalysisLaneSpec]
    Group --> Outputs[vector AnalysisOutputSpec]
    Lanes --> DerivedSignal[mid / side / left / right]
    Outputs --> TraceKind
    Outputs --> OutputMixMode
```

Current planning rules:

- `Mid`
  - one lane: `DerivedSignal::mid`
  - one output: `OutputMixMode::singleLane`
- `Side`
  - one lane: `DerivedSignal::side`
  - one output: `OutputMixMode::singleLane`
- `Stereo`
  - two lanes: `DerivedSignal::left`, `DerivedSignal::right`
  - one output: `OutputMixMode::averagePower`

## 7. SourceSet To AnalysisGroupProcessor To RawTrace

```mermaid
flowchart LR
    SourceSet[SourceSet\nblock-local signal views] --> Select[AnalysisGroupProcessor selects lane sources]
    Select --> Filter[band filters process selected lanes]
    Filter --> Mix[AnalysisOutputSpec mixes lane powers]
    Mix --> Measurements[outputMeasurements]
    Measurements --> RawTrace[writeRawTraces writes into published RawTrace slots]
```

Plainly:

- `SourceSet` says where the samples for this block live.
- `AnalysisGroupProcessor` reads one `AnalysisGroupSpec` and processes the requested lanes.
- `RawTrace` is the published per-band result for one slot trace.

Examples:

- In `mid` mode, `mainMid` or `sidechainMid` feeds one lane and produces one slot trace.
- In `side` mode, `mainSide` or `sidechainSide` feeds one lane and produces one slot trace.
- In `stereo` mode, `left` and `right` feed two lanes and produce one slot trace by averaged power.

## 8. Publish Path

```mermaid
flowchart LR
    ProcessorState[AnalysisGroupProcessor outputMeasurements] --> Write[writeRawTraces]
    Write --> Writer[TripleBuffer writer storage]
    Writer --> Publish[traces.publish]
```

- The engine writes processor outputs directly into pre-sized triple-buffer writer storage.
- There is no temporary per-block `traceScratch` vector in the publish path.

## 9. UI Data Flow

```mermaid
flowchart TD
    Engine[Analyzer::Engine] --> BandInfo[shared_ptr vector BandInfo]
    Engine --> RawTraces[vector RawTrace]

    Processor[SpectrumAnalyzerAudioProcessor] --> Slots[Ui::SignalSlotState array]
    Processor --> Freeze[freeze]
    Processor --> Meter[MeterSettings]
    Processor --> Grid[grid settings]
    Processor --> Sidechain[sidechain availability]

    BandInfo --> DataSource[AnalyzerDataSource]
    RawTraces --> DataSource
    Slots --> DataSource
    Freeze --> DataSource
    Meter --> DataSource
    Grid --> DataSource
    Sidechain --> DataSource

    DataSource --> AnalyzerComponent
    DataSource --> SignalRack[signal rack UI]
    DataSource --> MeterControls[meter toggle UI]
```

## 10. Current State

- The backend is slot-based, not single-mode based.
- `mid`, `side`, and `stereo` are implemented for both main input and sidechain input.
- Sidechain bus support is implemented in the processor and source builder.
- The analyzer UI supports up to `4` signal slots, each with:
  - enabled state
  - visibility
  - source
  - mode
  - color
  - opacity
- Global freeze exists as a UI-facing parameter/state.
- Meter visibility toggles (`Peak`, `RMS`, `Hold`) are UI-controlled existing parameters.

## Guiding Rule

The host owns the incoming `AudioBuffer`.
The engine owns derived per-block buffers and processor state.
`SourceSet` is only a set of lightweight views for the current block.
The processor bridges APVTS-backed settings into:

- engine-facing slot configuration for audio-thread processing
- UI-facing slot presentation and analyzer display state for the editor
