# Windows Profiling Session — Handoff (60 minutes, hard limit)

You are profiling the `band-spectrum-analyzer` VST3 on this Windows machine. Everything you need
is in this file; the full background is in `docs/performance-optimization-plan.md` (§1b holds
yesterday's Windows baseline). **Measurement and analysis only — no code changes, no
optimization.**

## Context (30 seconds)

Yesterday's baseline (Bitwig 5.3.8, per-plugin sandboxing, steady loud loop, 1 stereo slot,
Peak + RMS + Hold, editor visible, 1x, i7-8700 / GTX 1070 Ti / 1080p @ 143 Hz / 100% scaling):

- Ours ~12.4% of one core, SPAN ~12.8%, TDR Nova ~5.0%. Idle floor: ours ~1.2%, others ~0%.
- GPU-engine work ~2% and flat across UI scales → the remaining cost is CPU-side paint-pipeline
  work on the message thread. Which part, exactly, is unknown — that's this session.
- Run averages oscillate ±4–5 pts (σ≈4) with a multi-second period on the *steady* loop —
  unexplained. Second target of this session.
- Not yet measured: editor-closed (engine-only) cost. First target — it decides how much of the
  12.4% is display at all, and whether AVX SIMD work on the filterbank matters on Intel.

## Ground rules (from the W6 protocol — violations invalidate the run)

- CI **RelWithDebInfo** VST3, never Debug. PDB matching the loaded binary if available; if there
  is no PDB, proceed anyway — module-level attribution still works, don't burn time rebuilding.
- Steady **loud loop** on repeat. Never dynamic material (drifts averages 1.5–2.5 pts).
- 1 stereo slot; **verify Peak + RMS + Hold are actually ON** in the measured instance (a fresh
  insert once shipped with RMS off and nearly invalidated a run).
- Editor fully visible and unoccluded, 1x UI scale, Windows display scaling 100%, mouse away
  from the plot.
- Bitwig per-plugin sandboxing on ("individual plug-in processes") → our sandbox PID is one of
  the `BitwigPluginHost-X64-SSE41.exe` processes. Map PID → plugin via
  `tasklist /m band-spectrum-analyzer.vst3` (the module inside the .vst3 bundle carries the
  .vst3 extension), or Process Explorer's module list.
- The measurement script from yesterday is `scripts/perf/measure-plugin-cpu.ps1` on this
  machine. **If it is not committed to the repo, commit and push it before the session ends.**
  If you can't find it, use the fallback snippet below.

Fallback per-PID measurement (note: `$pid` is reserved in PowerShell, use another name):

```powershell
$procId = <PID>
$p = Get-Process -Id $procId; $t0 = $p.TotalProcessorTime; $w0 = Get-Date
Start-Sleep -Seconds 60
$p = Get-Process -Id $procId; $t1 = $p.TotalProcessorTime; $w1 = Get-Date
"{0:N1} % of one core" -f (($t1 - $t0).TotalSeconds / ($w1 - $w0).TotalSeconds * 100)
```

## Priorities — if time runs out, deliver in this order

1. **P0 — editor-closed engine-only cost** (one number; decomposes the 12.4%)
2. **P1 — ETW attribution of the message-thread paint cost** (module level is enough)
3. **P2 — explain the ±4–5 pt multi-second oscillation** (same trace)
4. **Stretch — idle-floor attribution** (tail of the same trace) and/or a 2x-scale capture

## Timeline

### T+0–10 — Setup

1. Bitwig session per the ground rules; loop playing; our editor open at 1x; RMS/Peak/Hold
   verified ON.
2. Identify our sandbox PID (see above). Note it.
3. Check the PDB: is there a `.pdb` next to (or built with) the exact VST3 binary loaded? Note
   yes/no and its path. Do not rebuild.

### T+10–20 — P0: control run + editor-closed run

1. **Control** (editor open, playing): measure 40 s. Expect ~12.4%. If off by more than ~3 pts,
   re-check the ground rules once, note the discrepancy, and continue — don't chase it.
2. **Editor closed** (close only the plugin window; keep the loop playing; sandbox PID
   survives): measure 60 s. **This is the single most important number of the session.**
3. Reopen the editor, confirm CPU returns to roughly the control value (one 30 s check).

### T+20–30 — ETW capture

`wpr.exe` ships with Windows 10 — no install needed for capture.

```powershell
mkdir C:\perf -ErrorAction SilentlyContinue
wpr -start CPU -filemode
# now: 90 s of the steady loop playing, editor open at 1x, hands off the machine
# then: stop playback in Bitwig, leave the editor open, wait 30 more s (idle-floor tail)
wpr -stop C:\perf\bsa-profile.etl
```

One trace answers P1, P2, and the idle-floor stretch goal. Keep the .etl regardless of how far
the analysis gets.

### T+30–50 — P1: attribution in WPA

WPA install if missing: `winget install Microsoft.WindowsPerformanceAnalyzer` (Microsoft
Store package). **Timebox all tooling problems to 10 minutes.** If WPA can't be installed in
time: use Superluminal if present; otherwise fall back to Process Explorer → our PID →
Threads tab (per-thread CPU + start address) which at least splits message thread vs display
worker vs audio thread, and note that the .etl is saved for later analysis.

In WPA:

1. Symbols: point at our PDB only — set `$env:_NT_SYMBOL_PATH = "<pdb folder>"` *before*
   launching WPA from that shell, then Trace → Load Symbols. **Do not add the Microsoft symbol
   server** — downloads can eat 15 minutes and module-level granularity answers P1 without OS
   symbols.
2. Graph: CPU Usage (Sampled) → filter to our sandbox PID → group by Thread, then Module, then
   Function/Stack.
3. Answer, with rough % of the PID's total CPU:
   - Split by thread: message/paint thread vs display worker vs audio thread vs anything else.
   - On the message thread, split by module: our module (`band-spectrum-analyzer.vst3`) vs
     `d2d1.dll` / `d3d11.dll` / `dxgi.dll` vs JUCE software rasterization vs kernel/waits.
   - Inside our module (needs the PDB): how much is path/batch building
     (`AnalyzerRenderBatchBuilder`, stroke generation) vs everything else.
4. Deliverable: a top-10 table (thread / module / function, % of PID CPU).

### T+50–60 — P2: the oscillation

1. In the same trace, plot CPU usage over time for our PID across the 90 s playing segment.
   Confirm the multi-second swing is visible; note its period.
2. Zoom one ~5 s HIGH window and one ~5 s LOW window; compare their sampled-stack profiles:
   - Same stacks, uniformly scaled up → content/work-driven (more path work per frame).
   - Different stacks (D2D flush, present/DWM waits, kernel time) → pipeline/timing-driven.
     One named hypothesis: 50 fps publish phase-drifting against the 143 Hz refresh.
3. Deliverable: one paragraph — period, verdict (work-driven vs timing-driven), and the top
   differing stack.

### Stretch (only if everything above is done)

- Idle floor: analyze the 30 s tail of the trace — what are the 1.2%'s stacks?
- 2x capture: set UI Scale to 2x, capture 60 s, check which stacks grew vs 1x (explains why the
  1.5x→2x segment steepens).

## Deliverables checklist

- [ ] Control %, editor-closed %, reopen-check % (P0)
- [ ] Thread/module/function attribution table (P1)
- [ ] Oscillation paragraph: period + verdict + evidence (P2)
- [ ] Idle-floor stacks and/or 2x diff (stretch, optional)
- [ ] `C:\perf\bsa-profile.etl` kept on disk
- [ ] `scripts/perf/measure-plugin-cpu.ps1` committed and pushed if it wasn't already
- [ ] Findings written into `docs/performance-optimization-plan.md` §1b (or a new
      `docs/windows-profile-findings.md` if that's faster), committed and pushed

## Non-goals for this hour

No optimization, no code changes, no re-measurement of the 1x/1.5x/2x curve, no SPAN/Nova
re-runs, no Debug builds, no Microsoft symbol server.
