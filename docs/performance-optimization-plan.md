# Performance Optimization Plan

Goal: make display cost independent of UI scale and pixel density, while keeping the 50 fps
evenly-paced display and the current 1x efficiency lead, on every platform we ship: VST3 on macOS,
Windows, and Linux (plus AU/Standalone on macOS).

Measured reality (2026-07-05, see §1): at 1x on macOS we are already the cheapest analyzer measured
on the reference machine — ~6% of one core in-DAW vs SPAN's ~29% and TDR Nova's ~12%. The problem is
the scale curve: display cost is linear in pixel area, so 1.5x on a retina display costs ~15% and 2x
extrapolates to ~25%, which erases the lead exactly in the configuration a laptop user would pick.
The historical "we are slower than SPAN" impression traces to scaled-up retina rendering, not to any
inefficiency at 1x. This is a rendering-architecture problem (who owns the plot's pixels), not a DSP
problem.

Platform reality check: every number in §1 is macOS-only. The three platforms have completely
different rendering economics and must each be measured before optimized:

- macOS: CoreGraphics drawing is GPU-executed; the cost is CoreAnimation layer/commit churn,
  proportional to invalidation rate × pixel area. Measured.
- Windows: JUCE 8 renders through Direct2D by default (GPU-backed) — confirmed in practice
  (§1b): the scale curve is far flatter than macOS (1.87× at 4× pixels vs 4.2×), rasterization is
  on the GPU, and the remaining cost is CPU-side paint-pipeline work — W2's territory. Measured.
- Linux: X11 software rasterization pays full CPU price for drawing complexity and scales with
  pixels at least as badly as macOS. Likely our worst platform. Unmeasured.

## 1. Current State (macOS, measured 2026-07-05)

Machine: 14" MacBook Pro (Apple Silicon, ProMotion, retina). RelWithDebInfo builds, `sample` +
`top` over 30–40 s per data point.

Standalone, 48 kHz input, 3 slots, Peak + RMS + Hold, 1x:

| Scenario | CPU (one core) |
|---|---|
| Display at 62.5 fps (original) | ~16.6% |
| Display at 50 fps evenly paced (current) | ~13.7% |
| Frozen / silent idle floor | ~4.2% |
| Editor closed (engine only, estimated) | ~1% |

In-DAW comparison (Bitwig, per-plugin sandboxing → one PID per plugin; same song, editors visible,
1 stereo slot, RMS + Hold on, 1x):

| Plugin | CPU (one core) |
|---|---|
| band-spectrum-analyzer | ~5.6% |
| TDR Nova | ~12% |
| SPAN | ~29% |

Cross-checked in Reaper (plugin bridged to `reaper_host_arm64`, own PID): ~6.4% — bridge overhead
is negligible. SPAN's cost is dominated by CPU-rasterized full-display texture uploads through
CoreAnimation (classic software-renderer architecture, punished by modern retina compositing).

UI-scale curve (Reaper-hosted, 1 slot, RMS on; "pixels" = points × display backing scale):

| Configuration | Pixels vs 1x-retina | CPU (one core) |
|---|---|---|
| 1x on retina, or 2x on a 1x-density monitor | 1× | ~6% |
| 1.5x on retina | 2.25× | ~15.1% |
| 2x on retina (extrapolated, linear fit) | 4× | ~25% |

Attribution of the display pipeline (call-tree samples, standalone):

- The dominant cost is CoreAnimation commit machinery — full-plot layer invalidation per frame
  drives backing-store texture swaps, IOSurface churn, and message-thread stalls waiting on the
  Metal-backed CG queue. Scales linearly with pixel area (measured) and frame rate (measured).
- ~1.5 pts at 1x: our paint code (of which ~1 pt is the double-stroked curved RMS paths)
- ~1 pt: display worker (meter/hold math, per-tick allocations)
- Not display: ~1 pt engine DSP (3 slots; ~0.2% with 1 slot), ~3 pts CoreAudio/standalone overhead

Key insights from profiling: CG rasterization is already GPU-executed on modern macOS
(`CA::CG::AccelQueue`); drawing complexity barely matters — **invalidation rate × pixel area is the
bill**, and UI scale multiplies pixel area. Frame pacing must stay even — irregular publish
intervals (hop-synced 16/32 ms alternation) produced visible judder and were rejected. Window
occlusion suppresses painting, so all benchmarks require fully visible editors.

## 1b. Current State (Windows, measured 2026-07-06)

Machine: desktop i7-8700 (6C/12T; roughly half the single-core speed of the M-series reference
laptop), GTX 1070 Ti, 1920×1080 @ 143 Hz, Windows 10, display scaling 100% — pixels = points, so
the UI Scale parameter is the only pixel multiplier (no retina-style DPI trap on this setup).
Bitwig 5.3.8 with per-plugin sandboxing (`BitwigPluginHost-X64-SSE41.exe`, one PID per plugin),
CI RelWithDebInfo VST3, JUCE 8.0.12 (Direct2D renderer active — verified, see GPU note below).
Measured with `scripts/perf/measure-plugin-cpu.ps1` (per-PID `TotalProcessorTime` deltas, 40–60 s
per point, reported as % of one core), steady loud loop.

In-DAW comparison (Bitwig, one track, editors fully visible, 1 stereo slot, Peak + RMS + Hold, 1x):

| Plugin | CPU (one core) |
|---|---|
| band-spectrum-analyzer | ~12.4% |
| SPAN | ~12.8% |
| TDR Nova | ~5.0% |

Idle floor (playback stopped, editors visible, 1x): ours ~1.2%, TDR Nova ~0.1%, SPAN 0.0% — SPAN
stops repainting entirely when its display decays. Context: Bitwig's own UI process runs ~36% of a
core with three editors open; its audio-engine process ~1.5%.

UI-scale curve (ours via the UI Scale parameter; SPAN and Nova unchanged across runs — valid
controls):

| Configuration | Pixels vs 1x | CPU (one core) | vs 1x cost |
|---|---|---|---|
| 1x | 1× | ~12.4% | 1× |
| 1.5x | 2.25× | ~15.7% | ~1.27× |
| 2x | 4× | ~23.2% | ~1.87× |

**The W6 question is answered: Direct2D gives a decisively flatter scale curve than CoreGraphics**
— 1.87× at 4× pixels vs the 4.2× extrapolated on macOS. W1 Phase 2 does not need a native Windows
backend. Note the curve is not perfectly flat and steepens between 1.5x and 2x (+2.6 pts per 1x
area unit on the first segment, +4.3 on the second).

GPU attribution (per-process GPU-engine counters via `typeperf`): our sandbox PID submits real
3D-engine work — ~1.8% / 2.0% / 2.3% at 1x / 1.5x / 2x — near-flat in pixel area, with huge
headroom. SPAN and TDR Nova submit zero GPU work (CPU-rendered on Windows too). The ETW profile
(2026-07-07, `docs/windows-profile-findings.md`) attributed the remaining CPU cost: our own code
is <1 pt — the bill is JUCE's D2D backend machinery, the GPU driver and kernel time, so W2 will
not move Windows; the levers here are fewer primitives/frame or a lower present rate (W1 seam).

Windows-specific observations:

- Cost tracks content, not pixels: on dynamic material our cost swung ~2–18% with the music
  (silent passages draw near-flat, cheap paths). The steady-loop ±4–5 pt oscillation (σ≈4) was
  profiled 2026-07-07: work-driven amplitude of the identical pipeline (uniform stacks in high
  and low windows), sub-5 s period; a 50 fps-vs-143 Hz vsync beat was ruled out.
- SPAN's repaint-skipping (hard 0% windows mid-song on dynamic material) flatters its averages on
  real songs; ours keeps painting at ~1.2% when silent. W4's occlusion work could borrow the idea
  as a silence gate.
- Absolute numbers do not compare across machines — this core is ~half an M-series core, so our
  12.4% here is consistent with the 5.6% on macOS. Within-machine ratios are the signal: we tie
  SPAN at 1x instead of leading 5×, plausibly because 1080p at 100% scaling gives SPAN's software
  renderer a small pixel bill, while retina charged it 4× that.
- Editor-closed (engine-only) cost: measured 2026-07-07 at ~0.5% (single active thread — the
  display worker stops headless). Display is ~96% of the in-DAW cost; full attribution in
  `docs/windows-profile-findings.md`. Still unmeasured: the standalone build.

## 2. Constraints

- Architecture boundaries stay: `dsp` / `display` / `plugin` / `ui`, both triple buffers unchanged.
- No visual regressions: 50 fps evenly paced is the accepted floor; no user-facing FPS setting.
- Every change must build and behave on macOS, Windows, and Linux VST3. Platform-specific rendering
  backends are allowed only behind the renderer seam defined in W1; feature code stays portable.

## 3. Workstreams

### W1 — GPU plot surface (the structural fix: flattens the scale curve)

The plot must stop being a rasterized, fully re-uploaded surface every frame. GPU geometry cost is
nearly independent of backing scale — this is the only workstream that fixes the measured 4× cost
at 2x-on-retina rather than shaving points off it. At 1x it additionally recovers most of the
CoreAnimation commit overhead. Cross-platform strategy, in order of preference:

- Phase 0, renderer seam: isolate plot rendering behind a small interface in
  `ui/analyzer/plot/render/` (batches in, pixels out) so backends can differ per platform without
  touching feature code. Tooltip and axis labels stay on the JUCE software path everywhere — they
  repaint rarely and cost nothing.
- Phase 1, `juce::OpenGLContext` on the plot (all three platforms, one code path): attach,
  re-measure per platform. GL is deprecated-but-shipping on macOS and fully supported on
  Windows/Linux; it is the only built-in JUCE GPU path that covers all targets. Decision gate per
  platform: keep GL where it wins, keep the software path where it doesn't.
- Phase 2, only where measurement demands it: platform-native backend behind the same seam —
  Metal (`CAMetalLayer`) on macOS if/when GL removal becomes real; on Windows verify JUCE 8's
  default Direct2D renderer first, it may already deliver the target with zero work; on Linux GL
  is the only realistic accelerated option.
- Phase 3, hosted validation on all platforms: VST3 in Ableton/Reaper/Bitwig (macOS + Windows +
  Linux), AU in Logic — window moves, UI-scale changes, occlusion, multiple instances, X11
  compositor variations (GNOME/KDE, XWayland).

Target: display pipeline ≤3 pts at 50 fps on each platform, **within ~1.5x of that at 2x-on-retina**
(acceptance is the scale curve, not the 1x number — 1x is already won).

### W2 — Paint content efficiency (~1–1.5 pts on macOS, likely the biggest lever on Linux)

Platform-universal: this work benefits the software path directly and shrinks GPU batch building
after W1. On Linux, where every stroke is CPU-rasterized, expect a much larger relative win than
macOS measured — do this before any Linux-specific rendering work.

- Done: replaced the underlay+main double stroke on RMS lines with one slightly heavier,
  higher-contrast coloured stroke.
- Reuse batch storage across frames: keep `RectangleList`/`Path`/point-vector capacity alive in
  `AnalyzerRenderBatchBuilder` instead of destroy-and-reallocate per frame.
- Acceptance: paint ≤0.7% in a sample profile on macOS; equivalent relative reduction confirmed in
  a Linux `perf` profile; zero steady-state allocations in the paint path.

### W3 — Worker efficiency (~0.5 pts)

- Done: reuse `MeterTrace` vectors across ticks instead of constructing fresh ones per trace per
  tick; regression coverage verifies stable storage for an unchanged layout.
- Write slot frames directly into the triple-buffer writer storage instead of copying vectors
  through `assignSlotFrame`.
- Acceptance: worker ≤0.5% active in profile; zero steady-state allocations on the worker thread.

### W4 — Cadence and presentation polish

- Keep the fixed 20 ms (50 fps) evenly-paced tick as the only active mode. Implemented with
  monotonic absolute deadlines: work time does not extend the interval, missed deadlines are
  skipped instead of replayed, immediate control refreshes reanchor the schedule, and global
  freeze waits without polling. Freeze transitions are handled before waiting so time spent
  frozen cannot advance meter or hold decay on resume. `Display::Constants::framesPerSecond` in
  `display/analyzer/config/AnalyzerDisplayConstants.h` is the single code-level rate setting;
  scheduling derives its interval from that value.
- Once W1 lands, consider presenting on the display's vblank (`VBlankAttachment`) so publish and
  present never beat against each other.
- Investigate window-occlusion detection (editor hidden behind DAW windows) to pause repaints the
  same way the settings-page gate already does. Per-platform: NSWindow occlusion state on macOS,
  `DwmGetWindowAttribute`/cloaking heuristics on Windows, visibility events on X11 — implement
  wherever the platform makes it reliable, skip where it does not.

### W5 — Audio-thread hygiene (policy, not CPU)

- Move engine rebuilds off the audio thread: parameter changes currently trigger
  `rebuildProcessors()`/`rebuildBands()` (allocations, filter re-preparation) inside
  `processBlock`. Prepare the new processor set on the message thread and hand it over through a
  lock-free slot; also removes the momentary all-slot trace blank on any engine-parameter change.
- `FilterBank` x86 SIMD width (existing TODO): Windows/Linux Intel builds currently get 4-wide SSE
  via `juce::dsp::SIMDRegister`; AVX runtime dispatch would halve filterbank cost there. Worth doing
  once per-platform baselines exist — the filterbank is a bigger share of total cost on machines
  without Apple-Silicon-class single-core performance.
- Decided and documented: the output mixer keeps its own mid/side derivation (independence from
  engine gating is worth the duplicated per-sample add/multiply while soloing).

### W6 — Benchmark and regression protocol (do first, on every platform)

- Canonical bench: RelWithDebInfo standalone (or Reaper-hosted VST3 where standalone is
  unavailable), 48 kHz, a fixed bundled test loop at a fixed level (mic input caused ±1.5 pt
  variance between runs today — not repeatable), 3 slots, everything on, 1x scale, window fully
  visible, mouse away from the plot.
- Per-platform tooling: macOS `top`/`sample` (used for §1); Windows Task Manager per-process % plus
  an ETW-based profiler (Superluminal, WPA, or VS Profiler); Linux `top`/`pidstat` plus
  `perf record -g`. Real hardware only — VMs have no representative GPU path.
- Reference plugins: SPAN (and TDR Nova if installed) on the same machine, same host, same source,
  default settings.
- Budgets (regression gates): macOS 1x total ≤7% in-DAW (measured 5.6%), 1.5x ≤16%; after W1 the
  scale curve must flatten (2x-on-retina ≤1.5× the 1x cost). Idle floor ≤4.5% standalone.
  Windows (i7-8700 reference, in-DAW): 1x ≤14% (measured 12.4%), 2x ≤26% (measured 23.2%), idle
  floor ≤2% (measured 1.2%). Linux budgets set once its baseline exists.

Windows baseline protocol (done 2026-07-06, results in §1b; tooling:
`scripts/perf/measure-plugin-cpu.ps1`; kept for re-runs — lesson learned: the steady loud loop is
mandatory, dynamic material drifted run averages by 1.5–2.5 pts and let SPAN idle-skip mid-song):

1. Build the VST3 in RelWithDebInfo (symbols matter for sampling; never benchmark Debug).
2. Bitwig with per-plugin sandboxing ("individual plug-in processes") — each plugin gets its own
   PID, exactly like the macOS runs. Load our plugin and SPAN on the same track, same test song,
   both editors fully visible and unoccluded, 1 stereo slot, Peak + RMS + Hold on. Verify RMS is
   actually enabled in the instance being measured (a fresh insert once shipped with it off and
   nearly invalidated a run).
3. Per-PID CPU: Task Manager details view or
   `Get-Counter '\Process(BitwigPluginHost*)\% Processor Time'` averaged over ≥30 s; map PIDs to
   plugins via loaded-module lists (Process Explorer or `tasklist /m`). Note core count — normalize
   to "% of one core" for comparison with §1.
4. Data points, in order: ours at 1x, ours at 1.5x (set via the UI Scale parameter from the host —
   record the display's DPI scaling, since pixels = points × scale factor, same trap as retina),
   SPAN at its default. Optional: ours at 2x if the screen fits it fully.
5. Deliverable: the §1 tables reproduced for Windows, plus a note on whether D2D gives a flatter
   scale curve than CoreGraphics did — that answer decides whether W1 Phase 2 targets Windows at
   all.

## 4. Sequencing

1. W6 — macOS baseline done (§1), Windows baseline done (§1b, D2D verdict: no native Windows
   backend needed). Next: Linux (needs the hardware)
2. W2 + W3 — platform-universal quick wins (one day); larger relative win expected on Linux
3. W1 Phases 0–1 — renderer seam + GL experiment, measured per platform against the scale curve,
   decision gates (1–2 days)
4. W1 Phases 2–3 — native backends only where the gates demand them (up to 1–2 weeks per backend)
5. W4 — occlusion + vsync polish (1–2 days)
6. W5 — engine rebuild handoff plus x86 SIMD dispatch, guided by the Intel baselines

Expected end state: scale-independent display cost — ≤7% of one core in-DAW at any UI scale on
macOS, Windows and Linux inside their own budgets, and the existing 1x lead over the reference
plugins preserved everywhere.

## 5. Non-goals

- DSP algorithm rewrites (engine is ~1% on Apple Silicon; SIMD width on Intel is W5, not a rewrite)
- User-facing frame-rate settings (rejected: one good default)
- Dirty-rect repainting on the software path (bars span the full plot width; W1 obsoletes it)
- Wayland-native or Vulkan backends (X11/GL covers Linux hosts today; revisit if hosts move)
