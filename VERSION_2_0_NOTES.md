# LFOz Tool Version 2.0 Branch

Branch name in the local workspace:

```text
version-2.0
```

Author: **lv601**

## Implemented in 2.0

### Curve editor

- Interactive curve editor in the main display.
- Click in the editor to add points.
- Drag points to move them.
- Double-click a point to delete it.
- Maximum 16 curve points.
- Curve data is saved/restored with the plugin state.
- New waveform type: `CURV`.

### Curve interpolation modes

- Smooth
- Step
- Bezier-style smoothstep interpolation

### FL Studio modulation routing improvements

- New output mode parameter:
  - `ADD`: add LFO/CV to incoming audio.
  - `CV ONLY`: output only the LFO/CV signal, no passthrough.
- Default output mode is `CV ONLY`, which is better for FL Studio Patcher / Fruity Peak Controller workflows.
- Bipolar output is limited to `-1..+1`.
- Unipolar output is limited to `0..1`.

### BPM/sync improvements

Expanded sync divisions:

- Free
- 1/16
- 1/16T
- 1/16D
- 1/8
- 1/8T
- 1/8D
- 1/4
- 1/4T
- 1/4D
- 1/2
- 1 Bar
- 2 Bar

### Host transport improvements

- New Phase Lock parameter.
- When Phase Lock is enabled and the host transport is running, synced LFO phase follows the DAW PPQ position.
- Transport start and large host timeline jumps retrigger/snap the LFO more accurately.

## Build

Use the existing Windows build flow:

```bat
cmake --preset vs2022-x64
cmake --build --preset release-vst3
```

or:

```bat
build_windows.bat
```
