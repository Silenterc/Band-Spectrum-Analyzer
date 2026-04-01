# Band Spectrum Analyzer
An analog-inspired fractional-octave spectrum analyzer plugin. It supports displaying up to four signals at once, with Peak, RMS, Peak Hold, and Freeze controls.


<img width="998" height="597" alt="Screenshot1 4" src="https://github.com/user-attachments/assets/1b80cdd6-6b67-4117-b37a-6a73c0b9b8df" />

## Features
- Displays up to four signals at once
- Analyzes Main or Sidechain input in Mid, Side, or Stereo
- Supports 1/3, 1/4, 1/6, and 1/12 octave resolution, giving ~ 30 to 120 bands across a typical full-range display
- Can show Peak, RMS, and Peak Hold layers, with both global and per-signal Freeze
- Each signal's color, order, and opacity can be customized
- You can hover over the analyzer to see level in dB, frequency, and nearest note
- Runs as AU, VST3, and Standalone
- Uses a bandpass filterbank rather than FFT
