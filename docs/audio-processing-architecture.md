# Audio Processing Architecture

This document describes the current audio-processing/backend architecture from `SpectrumAnalyzerAudioProcessor` down into the analyzer DSP engine and back up into the UI-facing analyzer data flow.

## 1. Audio Callback Flow

```mermaid
flowchart TD
    Host[Host audio block] --> Processor[SpectrumAnalyzerAudioProcessor]
    Processor --> Params[readCurrentParameterState]
    Params --> Engine[Analyzer::Engine]
    Processor -->|main buffer| Engine
    Processor -.->|future sidechain buffer| Engine
    Engine --> SourceBuilder[AnalysisSourceBuilder]
    Engine --> Processors[AnalysisGroupProcessor list]
    Processors --> Published[TripleBuffer vector RawTrace]
```

## 2. Engine Internals

```mermaid
flowchart TD
    Engine[Analyzer::Engine] --> Bands[shared band layout]
    Engine --> SourceBuilder[AnalysisSourceBuilder]
    Engine --> PlanBuilder[AnalysisPlanBuilder]
    Engine --> Processors[active AnalysisGroupProcessor list]

    SourceBuilder --> MainViews[mainLeft / mainRight]
    SourceBuilder --> MainMid[mainMidBuffer]
    SourceBuilder --> SideViews[sidechainLeft / sidechainRight]
    SourceBuilder --> SideMid[sidechainMidBuffer]

    PlanBuilder --> Specs[AnalysisGroupSpec list]
    Specs --> Processors
    Processors --> Mid[Mid processor]
    Processors --> Stereo[Stereo processor]
    Processors -.->|future| Sidechain[Sidechain processors]
    Processors -.->|future| Extra[Additional processors]

    Mid --> Published[TripleBuffer vector RawTrace]
    Stereo --> Published
    Sidechain -.->|future| Published
    Extra -.->|future| Published
```

## 3. UI Data Flow

```mermaid
flowchart TD
    Engine[Analyzer::Engine] --> BandInfo[shared_ptr vector BandInfo]
    Engine --> RawTraces[vector RawTrace]

    BandInfo --> DataSource[AnalyzerDataSource]
    RawTraces --> DataSource

    DataSource --> AnalyzerComponent
    AnalyzerComponent --> AnalyzerMeter
    AnalyzerMeter --> RenderData[RenderData / RenderTrace]
    RenderData --> AnalyzerViewModel
    AnalyzerViewModel --> Paint[paint]
```

## 4. Processor And Engine Types

```mermaid
flowchart TD
    Processor[SpectrumAnalyzerAudioProcessor] --> Engine[Analyzer::Engine]
    Processor --> DataSource[AnalyzerDataSource interface]
    Engine --> BandInfo[shared band layout]
    Engine --> SourceBuilder[AnalysisSourceBuilder]
    Engine --> PlanBuilder[AnalysisPlanBuilder]
    Engine --> Processors[vector AnalysisGroupProcessor]
    Engine --> Published[TripleBuffer vector RawTrace]
```

## 5. Engine Internals As A Tree

```text
SpectrumAnalyzerAudioProcessor
└── Analyzer::Engine
    ├── shared_ptr<vector<BandInfo>> bandInfo
    ├── AnalysisSourceBuilder sourceBuilder
    │   ├── SourceSet
    │   │   ├── mainLeft / mainRight
    │   │   ├── mainMid
    │   │   ├── sidechainLeft / sidechainRight
    │   │   └── sidechainMid
    │   └── engine-owned derived buffers
    │       ├── mainMidBuffer
    │       └── sidechainMidBuffer
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

## 6. Analysis Group Spec Model

```mermaid
flowchart LR
    AnalysisGroupSpec --> SourceFamily
    AnalysisGroupSpec --> Lanes[vector AnalysisLaneSpec]
    AnalysisGroupSpec --> Outputs[vector AnalysisOutputSpec]
    Lanes --> DerivedSignal
    Outputs --> TraceKind
    Outputs --> OutputMixMode
    Outputs --> LaneIndices[lane indices]
```

## 7. SourceSet To AnalysisGroupProcessor To RawTrace

```mermaid
flowchart LR
    SourceSet[SourceSet\nblock-local signal views] --> Select[AnalysisGroupProcessor selects lanes from SourceSet]
    Select --> Filter[process runs each band filter over the selected lanes]
    Filter --> Mix[AnalysisOutputSpec mixes lane powers into output powers]
    Mix --> Measurements[group.outputMeasurements]
    Measurements --> RawTrace[writeRawTraces writes directly into published RawTrace slots]
```

Plainly:

- `SourceSet` says where the samples for this block live.
- `AnalysisGroupProcessor` says which of those signals to read, how many lanes to process, and how outputs are mixed.
- `RawTrace` is the published per-band result after that processing is done.

Example:

- In `mid` mode, `SourceSet.mainMid` feeds one processor lane, and that processor publishes one `RawTrace`.
- In `stereo` mode, `SourceSet.mainLeft` and `SourceSet.mainRight` feed two lanes in one processor, and that processor still publishes one collapsed `RawTrace`.

## 8. Publish Path

```mermaid
flowchart LR
    ProcessorState[AnalysisGroupProcessor outputMeasurements] --> Write[writeRawTraces]
    Write --> Writer[TripleBuffer writer storage]
    Writer --> Publish[traces.publish]
```

- The engine no longer builds a temporary `traceScratch` vector per block.
- Each processor writes its outputs directly into pre-sized triple-buffer writer storage.
- This keeps the modular split without paying for an extra full-copy trace-materialization layer.

## Current State

- `mid` is implemented as one `AnalysisGroupProcessor` with one `mid` lane and one published `input` trace.
- `stereo` is implemented as one `AnalysisGroupProcessor` with `left` and `right` lanes and one published `input` trace mixed with `OutputMixMode::averagePower`.
- `side` is implemented as one `AnalysisGroupProcessor` with one `side` lane and one published `input` trace.
- Sidechain source plumbing exists, and sidechain-backed modes are available through the analysis mode selection.
- `SignalView` is just a lightweight pointer + length view.
  `left/right` point into host-owned buffers.
  `mainMidBuffer` / `sidechainMidBuffer` are engine-owned derived storage for this block.

## Guiding Rule

The engine owns derived per-block buffers and processor state.
The host owns the incoming `AudioBuffer`.
`SourceSet` is only a set of views that tells each active `AnalysisGroupProcessor` where to read its signal from for the current block.
