# Tracker DAW — Project Specification

## Project Name: **ChipForge**

A chiptune-focused tracker DAW inspired by the Polyend Tracker workflow and Renoise, built for Windows using JUCE/C++. Designed for composing 8-bit and 16-bit era video game music with a clean, SNES-inspired retro UI aesthetic.

---

## Vision

ChipForge combines the step-entry, pattern-centric workflow of hardware trackers (Polyend Tracker) with the power of desktop computing — more tracks, built-in chip-accurate synth emulation, bundled effects, sample manipulation, and a multi-window layout for composition, mixing, and arrangement. It is a composition and production tool, not a live performance tool.

---

## Technology Stack

### Core Framework
- **JUCE 8** (C++20) — industry-standard cross-platform audio framework
  - Handles: audio I/O (WASAPI/ASIO), MIDI, GUI rendering (Direct2D on Windows), DSP
  - GitHub: https://github.com/juce-framework/JUCE
  - License: AGPLv3 or commercial
  - Build system: CMake

### Why JUCE (not Tracktion Engine)
Tracktion Engine (https://github.com/Tracktion/tracktion_engine) provides a high-level DAW engine as a JUCE module, but it is designed around a traditional linear DAW paradigm (timeline, clips, tracks). ChipForge's tracker paradigm (patterns, steps, hex commands) is fundamentally different and would fight Tracktion Engine's architecture more than benefit from it. JUCE alone gives us the audio/MIDI/GUI foundation without imposing a DAW structure we'd have to work around.

### Sound Chip Emulation
- **Nes_Snd_Emu** (MVP) — NES 2A03 APU emulator
  - GitHub: https://github.com/jamesathey/Nes_Snd_Emu
  - High accuracy, LGPL licensed
  - Channels: 2x pulse, 1x triangle, 1x noise, 1x DPCM
- **Game Music Emu** (post-MVP reference) — multi-system emulation library
  - GitHub: https://github.com/libgme/game-music-emu
  - Covers: NES, SNES (SPC), Game Boy, Genesis, and more
  - LGPL licensed, C interface

### Effects DSP
JUCE includes built-in DSP modules for common effects (reverb, delay, chorus, phaser, filters, distortion, compressor). For MVP, these are sufficient. The JUCE DSP module provides:
- `juce::dsp::Reverb` — Freeverb-based algorithmic reverb
- `juce::dsp::DelayLine` — configurable delay
- `juce::dsp::LadderFilter` — classic resonant filter
- `juce::dsp::Chorus` — chorus effect
- `juce::dsp::Phaser` — phaser effect
- `juce::dsp::Compressor` — dynamics processing
- `juce::dsp::WaveShaper` — distortion/waveshaping

### Reference Projects (study, don't fork)
- **Furnace** (https://github.com/tildearrow/furnace) — the most comprehensive open-source chiptune tracker. Multi-system, uses quality emulation cores (Nuked, SameBoy, NSFplay, puNES, etc). Study its pattern format, effect command system, and instrument macro design. GPL v2. Written in C++ with Dear ImGui.
- **FamiStudio** (https://github.com/BleuBleu/FamiStudio) — NES-focused music editor with modern DAW-style UI. Uses Nes_Snd_Emu internally. Study its instrument envelope editor and QWERTY keyboard recording mode. C#, MIT license.
- **FamiTracker / Dn-FamiTracker** — the classic NES tracker. Study its effect command vocabulary (Bxx, Dxx, Fxx, etc.) which is the de facto standard for NES tracking.

### Build Environment
- **IDE**: Visual Studio 2022 (Windows primary target)
- **Build**: CMake (JUCE's recommended approach over Projucer for new projects)
- **Compiler**: MSVC (C++20)
- **Package management**: Git submodules for JUCE and emulation libraries

---

## Architecture

### Guiding Principles
1. **Audio thread isolation** — the real-time audio callback must never allocate memory, lock mutexes, or do any I/O. All communication between UI and audio uses lock-free queues/FIFOs.
2. **Modular engine** — the tracker engine (pattern playback, sequencing) is completely separated from the UI. This makes it testable and allows Claude Code to work on discrete modules.
3. **Data-driven patterns** — patterns are plain data structures. The engine reads them; the UI edits them. No tight coupling.
4. **Single-file modules where possible** — each major system (NES synth, sample engine, effect chain, pattern engine) should be a self-contained module with a clear header interface.

### High-Level Module Map

```
ChipForge/
├── CMakeLists.txt
├── libs/
│   ├── JUCE/                    # Git submodule
│   └── nes_snd_emu/             # Git submodule or vendored
├── src/
│   ├── main.cpp                 # JUCE app entry point
│   ├── App/
│   │   ├── ChipForgeApp.h/cpp           # JUCEApplication subclass
│   │   └── MainWindow.h/cpp             # Main window, manages views
│   ├── Engine/
│   │   ├── AudioEngine.h/cpp            # JUCE AudioAppComponent, audio callback
│   │   ├── TrackerEngine.h/cpp          # Pattern sequencer / playback
│   │   ├── MixerEngine.h/cpp            # Per-track volume, pan, effects routing
│   │   └── TransportState.h/cpp         # Play/stop/record/BPM/position
│   ├── Synths/
│   │   ├── SynthInterface.h             # Abstract base for all synths
│   │   ├── NES2A03/
│   │   │   ├── NES2A03Synth.h/cpp       # Wraps Nes_Snd_Emu as a playable instrument
│   │   │   └── NES2A03Voice.h/cpp       # Per-voice state (pulse1, pulse2, tri, noise)
│   │   └── SamplePlayer/
│   │       ├── SamplePlayer.h/cpp       # WAV sample playback engine
│   │       └── SampleVoice.h/cpp        # Per-voice: pitch, loop, volume envelope
│   ├── Effects/
│   │   ├── EffectChain.h/cpp            # Ordered chain of effects per track
│   │   ├── EffectInterface.h            # Abstract base for all effects
│   │   ├── ReverbEffect.h/cpp           # Wraps juce::dsp::Reverb
│   │   ├── DelayEffect.h/cpp            # Wraps juce::dsp::DelayLine
│   │   ├── FilterEffect.h/cpp           # Wraps juce::dsp::LadderFilter
│   │   ├── ChorusEffect.h/cpp
│   │   ├── DistortionEffect.h/cpp
│   │   └── BitCrusherEffect.h/cpp       # Custom: reduce bit depth/sample rate
│   ├── Data/
│   │   ├── Pattern.h/cpp                # Pattern data structure
│   │   ├── Song.h/cpp                   # Song = ordered list of pattern references
│   │   ├── Instrument.h/cpp             # Instrument definitions (synth params, sample refs)
│   │   ├── Project.h/cpp                # Top-level: song + instruments + samples + settings
│   │   └── ProjectSerializer.h/cpp      # Save/load to .cfp (ChipForge Project) format
│   ├── MIDI/
│   │   ├── MIDIInput.h/cpp              # MIDI controller input handling
│   │   └── MIDIExport.h/cpp             # Export patterns as MIDI files
│   ├── Export/
│   │   ├── WAVExporter.h/cpp            # Offline render to WAV
│   │   └── MP3Exporter.h/cpp            # Future: MP3 export via LAME
│   └── UI/
│       ├── LookAndFeel/
│       │   └── ChipForgeLookAndFeel.h/cpp  # Custom SNES-inspired theme
│       ├── TrackerView/
│       │   ├── TrackerView.h/cpp           # Main tracker grid (patterns/tracks)
│       │   ├── PatternEditor.h/cpp         # Individual pattern cell editing
│       │   ├── TrackHeader.h/cpp           # Track name, instrument select, mute/solo
│       │   └── StepCursor.h/cpp            # Current edit position
│       ├── MixerView/
│       │   ├── MixerView.h/cpp             # Channel strips view
│       │   ├── ChannelStrip.h/cpp          # Fader, pan, effects slots, meters
│       │   └── EffectSlotEditor.h/cpp      # Edit effect parameters
│       ├── ArrangementView/
│       │   ├── ArrangementView.h/cpp       # Horizontal timeline of pattern blocks
│       │   ├── PatternBlock.h/cpp          # Draggable pattern reference on timeline
│       │   └── TimelineRuler.h/cpp         # Beat/bar ruler
│       ├── InstrumentEditor/
│       │   ├── InstrumentEditor.h/cpp      # Envelope editors, synth params
│       │   └── EnvelopeEditor.h/cpp        # Visual envelope editing (volume, pitch, duty)
│       ├── SampleEditor/
│       │   ├── SampleEditor.h/cpp          # Waveform view, loop points, trim
│       │   └── SampleBrowser.h/cpp         # File browser for importing samples
│       └── Shared/
│           ├── VUMeter.h/cpp               # Level meter component
│           ├── Toolbar.h/cpp               # Transport controls, view switching
│           └── KeyboardInput.h/cpp         # QWERTY-to-note mapping (keyboard mode)
├── assets/
│   ├── fonts/                   # Custom pixel/retro fonts (OFL licensed)
│   └── icons/                   # UI icons
└── tests/
    ├── PatternTest.cpp
    ├── TrackerEngineTest.cpp
    └── SerializerTest.cpp
```

### Audio Thread Architecture

```
Audio Callback (real-time, ~5ms budget at 44.1kHz/256 samples)
│
├── TrackerEngine::processBlock()
│   ├── Advance step position based on BPM/tick
│   ├── For each track with a new note/command this step:
│   │   ├── Send note-on/off to assigned Synth or SamplePlayer
│   │   └── Process tracker effect commands (volume slide, arp, etc.)
│   └── Return per-track audio buffers
│
├── For each track:
│   ├── Synth/SamplePlayer::renderBlock() → raw audio
│   ├── EffectChain::processBlock() → processed audio
│   └── MixerEngine::applyGainPan() → stereo mix contribution
│
└── MixerEngine::sumToMaster() → final stereo output
```

### Lock-Free Communication (UI ↔ Audio)

```
UI Thread                         Audio Thread
    │                                  │
    ├── Edit pattern cell ──FIFO──→    │ (reads commands from queue)
    ├── Change BPM ─────────FIFO──→    │
    ├── Adjust fader ───────FIFO──→    │
    │                                  │
    │    ←──atomic reads──────────     ├── Current playback position
    │    ←──atomic reads──────────     ├── VU meter levels
    │    ←──FIFO──────────────────     ├── Trigger notifications
```

---

## Data Model

### Pattern
A pattern is a 2D grid: rows (steps) × columns (tracks). Each cell contains:

```
struct PatternCell {
    uint8_t  note;          // 0-127 (MIDI note) or NOTE_OFF or NOTE_EMPTY
    uint8_t  instrument;    // instrument index (0-255, 0xFF = no change)
    uint8_t  volume;        // 0-64 (0xFF = no change)
    uint16_t effectCommand;  // high byte = effect type, low byte = parameter
                            // e.g., 0x0C40 = set volume to 0x40
};
```

Default pattern length: 64 steps (configurable 1-256).

### Effect Commands (tracker-standard vocabulary)
These follow the FamiTracker/Renoise convention:

| Command | Name | Description |
|---------|------|-------------|
| `0xy` | Arpeggio | Cycle note, note+x, note+y each tick |
| `1xx` | Pitch slide up | Slide pitch up by xx per tick |
| `2xx` | Pitch slide down | Slide pitch down by xx per tick |
| `3xx` | Portamento | Slide to target note at speed xx |
| `4xy` | Vibrato | Speed x, depth y |
| `Axy` | Volume slide | Up x, down y per tick |
| `Bxx` | Jump to order | Jump to position xx in song order |
| `Cxx` | Set volume | Set volume to xx |
| `Dxx` | Pattern break | Jump to row xx of next pattern |
| `Fxx` | Set speed/tempo | xx < 0x20 = ticks/row, xx >= 0x20 = BPM |

### Song
A song is an ordered list of pattern references (the "order list"):
```
struct Song {
    std::string name;
    int bpm;
    int ticksPerRow;        // speed (typically 6)
    int rowsPerPattern;     // default 64
    std::vector<Pattern> patterns;      // pattern pool
    std::vector<std::array<int, 8>> orderList;  // each entry = which pattern index per track
};
```

### Project File Format (.cfp)
A `.cfp` file is a ZIP archive containing:
```
project.json          # metadata, song structure, order list, BPM, settings
instruments/          # one .json per instrument (synth params or sample references)
samples/              # .wav files referenced by sample-based instruments
```

JSON is used for human readability and easy debugging. Binary formats can be considered post-MVP for performance.

---

## UI Design

### Aesthetic: "American SNES"
The visual language draws from the SNES console era — the muted purples, grays, and blues of the American SNES hardware, combined with the clean pixel typography of 16-bit game menus.

**Color Palette:**
- Background: `#1A1A2E` (deep navy-purple, like the SNES console body)
- Panel backgrounds: `#16213E` (slightly lighter navy)
- Active/selected: `#533483` (SNES purple accent)
- Text primary: `#E0E0E0` (soft white, not harsh)
- Text secondary: `#8888AA` (muted lavender-gray)
- Highlight/cursor: `#7B2FF7` (vivid purple, the SNES power LED)
- Note on: `#00D4AA` (retro teal-green, common in 16-bit UIs)
- Warning/alert: `#FF6B6B` (soft red)
- Channel colors: a palette of 8 distinct muted retro colors for track differentiation

**Typography:**
- Use a proper pixel/bitmap font that evokes 16-bit game menus. Candidates (all OFL/free licensed):
  - **Press Start 2P** — iconic 8-bit style, good for headers
  - **Pixel Operator** — clean, readable at small sizes, good for tracker grid
  - **IBM Plex Mono** — more modern but retro-compatible, good for data-dense views
- The tracker grid specifically should use a monospace font for column alignment
- Avoid: system fonts, generic sans-serif, anything that looks like a typical modern app

**UI Details:**
- Beveled/embossed panel edges (subtle, like SNES menu chrome)
- No gradients or glass effects — flat with subtle depth cues
- Pattern cells should have thin grid lines (1px, low contrast)
- Selected row/cell highlighted with the purple accent
- Playback cursor as a full-width horizontal bar in the teal-green

### Window Layout

The application uses a tabbed or panel-switching approach for three main views, accessible via keyboard shortcuts or toolbar buttons:

#### 1. Tracker View (F1) — Primary Composition
```
┌─────────────────────────────────────────────────────┐
│ [Toolbar: Transport | BPM | Octave | Edit Step | View Tabs] │
├────┬────────────────────────────────────────────────┤
│    │  Trk1   │  Trk2   │  Trk3   │  Trk4   │ ...  │
│    │ NES Pls │ NES Pls │ NES Tri │ NES Noi │      │
│    ├─────────┼─────────┼─────────┼─────────┤      │
│ 00 │ C-4 01 │ --- -- │ E-3 03 │ --- -- │      │
│ 01 │ --- -- │ G-4 02 │ --- -- │ D#2 04 │      │
│ 02 │ D-4 01 │ --- -- │ --- -- │ --- -- │      │
│ 03 │ === -- │ --- -- │ F-3 03 │ --- -- │      │
│ >> │ E-4 01 │ A-4 02 │ --- -- │ --- -- │  ← cursor
│ 05 │ --- -- │ --- -- │ G-3 03 │ C#2 04 │      │
│ .. │         │         │         │         │      │
├────┴─────────┴─────────┴─────────┴─────────┤      │
│ [Pattern selector: 00 01 02 03 04 ...]      │      │
│ [Order list: current song position]          │      │
└─────────────────────────────────────────────────────┘
```
Each column shows: Note, Instrument, Volume (optional), Effect (optional).
`===` represents note-off. `---` represents empty.

#### 2. Mixer View (F2) — Mixing & Effects
```
┌──────────────────────────────────────────────┐
│  Track 1  │  Track 2  │  Track 3  │  ...     │
│  NES Pls1 │  NES Pls2 │  NES Tri  │          │
│           │           │           │          │
│  [FX1: Rev]│ [FX1: Dly]│ [FX1:Flt]│          │
│  [FX2: ---]│ [FX2: ---]│ [FX2:---]│          │
│  [FX3: ---]│ [FX3: ---]│ [FX3:---]│          │
│           │           │           │          │
│  ┃█████┃  │  ┃███  ┃  │  ┃████ ┃  │  VU     │
│  ┃█████┃  │  ┃███  ┃  │  ┃████ ┃  │ meters  │
│           │           │           │          │
│  [Pan: C ] │  [Pan: L ] │  [Pan: R ] │        │
│  [Vol ███] │  [Vol ██ ] │  [Vol ████]│ faders │
│  [M] [S]  │  [M] [S]  │  [M] [S]  │ mute/solo│
└──────────────────────────────────────────────┘
```

#### 3. Arrangement View (F3) — Song Structure
```
┌──────────────────────────────────────────────────┐
│ Bar: 1    2    3    4    5    6    7    8    ...  │
│ ─────────────────────────────────────────────     │
│ Trk1: [Pat00] [Pat00] [Pat01] [Pat01] [Pat02]   │
│ Trk2: [Pat00] [Pat03] [Pat01] [Pat04] [Pat02]   │
│ Trk3: [Pat05] [Pat05] [Pat06] [Pat06] [Pat07]   │
│ ...                                               │
│ ─────────────────────────────────────────────     │
│ Playback cursor ▼                                │
└──────────────────────────────────────────────────┘
```
Pattern blocks are draggable. Can copy/paste patterns. Supports snapping to pattern boundaries.

---

## Keyboard Input Mode

When "Keyboard Mode" is toggled (via a button or shortcut like `Space` or `` ` ``), the QWERTY keyboard maps to musical notes, similar to Renoise/FamiTracker:

```
Octave N:     Z=C  S=C# X=D  D=D# C=E  V=F  G=F# B=G  H=G# N=A  J=A# M=B
Octave N+1:   Q=C  2=C# W=D  3=D# E=E  R=F  5=F# T=G  6=G# Y=A  7=A# U=B
```

When a key is pressed:
1. The note is immediately played through the current track's synth/sample (audible feedback)
2. If recording is active, the note is written to the current pattern cell
3. The cursor advances by the "edit step" amount (configurable: 0, 1, 2, 4, etc.)

Octave is adjustable with `+`/`-` keys.

This is a core workflow feature — it makes the tracker feel like an instrument.

---

## MIDI Support

### Input
- MIDI note input works identically to keyboard mode — notes play and optionally record
- MIDI CC messages can be mapped to effect parameters in the Mixer view
- Uses JUCE's `MidiInput` class for device enumeration and callbacks

### Export
- Export song as a Standard MIDI File (.mid)
- Each track maps to a MIDI channel
- Pattern effect commands translated where applicable (volume, pitch bend)

---

## Sample Engine

The sample engine provides functionality comparable to what a 90s game soundtrack designer would have had:

### Sample Playback
- Load WAV files (8/16-bit, any sample rate — internally resampled)
- Pitch control: play at any note via resampling (linear interpolation, optionally sinc for quality)
- Loop modes: no loop, forward loop, ping-pong loop
- Loop point editor (start/end markers on waveform)

### Sample Manipulation
- Trim/cut (select region, delete or keep)
- Normalize (peak or RMS)
- Reverse
- Fade in/out
- Bit-depth reduction (for that crunchy retro feel)
- Sample rate reduction (downsample for lo-fi character)
- Basic pitch shift (resample-based)

### Sample Instruments
A sample-based instrument wraps a sample with:
- Volume envelope (ADSR or custom multi-point)
- Pitch envelope (for effects like pitch drops)
- Loop settings
- Base note (which note = original pitch)
- Fine tune (cents)

---

## Project Save/Load

### Format: `.cfp` (ChipForge Project)
A ZIP archive containing JSON + WAV samples. Human-readable, debuggable, version-control friendly (when unzipped).

### Auto-save
- Auto-save to a temp file every 2 minutes
- Crash recovery on next launch

### Export
- **WAV**: offline render of the full song to stereo WAV (44.1kHz/16-bit or 48kHz/24-bit)
- **MIDI**: export as .mid file
- **Per-track stems**: render each track to a separate WAV file

---

## Development Phases

### Phase 0 — Project Skeleton (Week 1-2)
**Goal**: Application launches, produces sound, can be built from source.

- Set up CMake project with JUCE as submodule
- Create `JUCEApplication` subclass with main window
- Implement basic audio callback that outputs a sine wave
- Apply custom LookAndFeel with SNES color palette
- Load and use a pixel font for the UI
- Basic toolbar with play/stop buttons
- Verify build works on Windows with Visual Studio 2022

**Deliverable**: App opens, you see the themed window, you hear a test tone when you press play.

### Phase 1 — NES Synth Integration (Week 3-4)
**Goal**: The NES 2A03 chip emulator is wrapped and playable.

- Integrate Nes_Snd_Emu as a library (vendored or submodule)
- Create `NES2A03Synth` class implementing `SynthInterface`
- Route note-on/off messages to APU register writes
- Expose parameters: pulse duty cycle (12.5%, 25%, 50%), noise mode (short/long)
- Implement keyboard input mode (QWERTY → notes → synth)
- Real-time audio playback of notes through JUCE audio callback

**Deliverable**: Press keys on your keyboard, hear NES sounds immediately.

### Phase 2 — Tracker Engine Core (Week 5-8)
**Goal**: You can create, edit, and play back patterns.

- Implement `Pattern` and `PatternCell` data structures
- Build `TrackerEngine` — steps through pattern rows at BPM/speed
- Implement basic tracker effect commands: set volume, note-off, arpeggio, pitch slide
- Build `TrackerView` UI — the pattern grid with cursor navigation
- Keyboard navigation: arrow keys to move cursor, enter/edit values
- Note entry via keyboard mode (type notes into cells)
- Play/stop with visual cursor tracking
- Pattern selector (switch between patterns)
- 8 tracks, each assignable to NES synth voices or sample player

**Deliverable**: Full tracker writing workflow — enter notes, play them back, hear the song.

### Phase 3 — Effects & Mixer (Week 9-11)
**Goal**: Per-track effects and mixing.

- Implement `EffectChain` with ordered insert effects
- Wrap JUCE DSP effects: reverb, delay, filter, chorus, distortion
- Build `BitCrusherEffect` (custom: bit depth + sample rate reduction)
- Build `MixerView` UI with channel strips
- Per-track: volume fader, pan knob, mute/solo, VU meter
- Effect slot UI: add/remove/reorder effects, edit parameters
- Master output metering

**Deliverable**: Tracks can be mixed and effects applied in real-time.

### Phase 4 — Sample Engine (Week 12-14)
**Goal**: Load and play samples, basic sample editing.

- Implement `SamplePlayer` synth (WAV loading, pitch interpolation, loop modes)
- Build `SampleEditor` UI (waveform display, loop point markers)
- Sample manipulation: trim, normalize, reverse, fade, bit-crush, downsample
- Sample-based instrument creation (ADSR envelope, base note, fine tune)
- Integration with tracker: tracks can use sample instruments

**Deliverable**: Load a WAV, pitch it to notes, sequence it in patterns.

### Phase 5 — Arrangement & Song Structure (Week 15-17)
**Goal**: Compose full songs with the arrangement view.

- Implement `Song` with order list (pattern sequencing)
- Build `ArrangementView` UI (horizontal timeline, draggable pattern blocks)
- Pattern copy/paste/duplicate in arrangement
- Song loop point
- Order list editing in tracker view (jump between patterns)
- Implement `Bxx` (jump) and `Dxx` (pattern break) commands

**Deliverable**: Arrange patterns into a complete song and play it through.

### Phase 6 — Project I/O & Export (Week 18-19)
**Goal**: Save/load projects, export audio.

- Implement `.cfp` project format (ZIP of JSON + samples)
- Save/load all song data, instruments, samples, mixer settings
- Auto-save with crash recovery
- WAV export (offline render)
- Per-track stem export
- MIDI export

**Deliverable**: Complete round-trip — create a song, save it, close the app, reopen, it's all there. Export to WAV.

### Phase 7 — MIDI Input (Week 20)
**Goal**: Use external MIDI controllers.

- MIDI device enumeration and selection (settings dialog)
- MIDI note input → plays synth + records to pattern
- MIDI CC mapping to mixer/effect parameters
- MIDI learn mode (click a parameter, move a knob, it's mapped)

**Deliverable**: Plug in a MIDI keyboard, play and record notes.

### Phase 8 — Polish & Instrument Editor (Week 21-23)
**Goal**: Refine the UI, add instrument editing depth.

- Instrument Editor panel: envelope editors for volume, pitch, duty cycle (macro-style, like FamiTracker)
- Visual envelope editor with draggable points
- Instrument presets (ship with a set of classic NES sound presets)
- Undo/redo system (command pattern)
- UI polish: animations, transitions, tooltips
- Keyboard shortcut reference / help overlay
- Settings dialog (audio device, buffer size, MIDI device, theme options)

**Deliverable**: A polished, usable application ready for testing.

---

## Future Milestones (Post-MVP)

These are explicitly out of scope for the initial build but documented for future planning:

### Additional Chip Emulation
- Game Boy DMG (4 channels: 2 pulse, 1 wave, 1 noise)
- SNES SPC700 (8 channels, BRR samples, echo/reverb DSP)
- Sega Genesis YM2612 (6 FM channels) + SN76489 (4 PSG channels)
- Reference: Game Music Emu covers all of these

### Purpose-Built Effects
- "VHS Tape" effect (wow/flutter, tape saturation, high-frequency roll-off — inspired by Baby Audio Super VHS)
- "CRT Hum" — subtle 60Hz hum and noise floor generator
- "SNES Echo" — authentic SNES-style reverb (8-tap FIR echo, 8-bit samples)
- "Chiptune Chorus" — pitch detuning that mimics the slight instability of old hardware

### Additional Features
- Third-party VST3 hosting (massive engineering lift)
- Cross-platform builds (macOS, Linux)
- Live performance mode
- Pattern visualization / oscilloscope view
- Plugin format export (ship ChipForge synths as VST3 plugins)
- Collaboration / project sharing features
- Tracker module import (.mod, .xm, .it)

---

## Debugging & Development Tooling

Real-time audio is the most debugging-intensive category of software. Clicks, pops, dropouts, and latency issues are common and notoriously subtle. The good news is the audio dev community has built excellent tools specifically for these problems. Integrate these from Phase 0 — don't wait until you hit issues.

### RealtimeSanitizer (RTSan) — The Most Important Tool

**What it is**: A sanitizer built into LLVM/Clang 20+ that detects real-time safety violations at runtime. It catches calls to `malloc`, `free`, `pthread_mutex_lock`, system calls, and anything else with non-deterministic execution time — but *only* when called from within a function you've marked as real-time.

**Why it matters**: The #1 cause of audio glitches is "something blocked the audio thread." RTSan catches this automatically. You mark your audio callback with `[[clang::nonblocking]]` and RTSan will immediately error if any code path from that function touches memory allocation, locks, or I/O.

**How to use it**:
```cpp
// Mark the audio callback as nonblocking
void processBlock(juce::AudioBuffer<float>& buffer) [[clang::nonblocking]] {
    // RTSan will catch any malloc, mutex, or system call in here
    // or in ANY function called from here
}
```

Compile with: `clang++ -fsanitize=realtime`

**Caveat for this project**: RTSan requires Clang and does not yet support Windows natively. For Windows/MSVC development, use it as a secondary CI check or test on a Linux/WSL build. The compile-time counterpart (`-Wfunction-effects`) can catch many issues statically.

- GitHub: https://github.com/realtime-sanitizer/rtsan
- Docs: https://clang.llvm.org/docs/RealtimeSanitizer.html
- ADC 2024 talk: "LLVM's Real-Time Safety Revolution" by David Trevelyan & Chris Apple

### Melatonin Perfetto — Audio Callback Profiling

**What it is**: A JUCE module that integrates Google's Perfetto performance tracing. Unlike traditional profilers that show averages (useless for audio — one 50ms spike in a 0.7ms budget causes a glitch), Perfetto shows every single audio callback on a timeline so you can spot outliers.

**Why it matters**: Standard profilers hide the exact problem you're trying to find. Perfetto answers: "Which specific audio callback took too long, and what was happening inside it?" You can see each `processBlock` call, how long it took, and drill into child function calls.

**How to use it**:
```cpp
#include "melatonin_perfetto/melatonin_perfetto.h"

void processBlock(juce::AudioBuffer<float>& buffer) {
    TRACE_DSP();  // That's it — this function is now traced
    // ... your audio code
}

void paint(juce::Graphics& g) override {
    TRACE_COMPONENT();  // Also trace UI paint calls
    // ... your paint code
}
```

The `TRACE_DSP()` and `TRACE_COMPONENT()` macros compile to no-ops when Perfetto is disabled (`PERFETTO=0`), so leave them in the codebase permanently.

- GitHub: https://github.com/sudara/melatonin_perfetto
- Guide: https://melatonin.dev/blog/using-perfetto-with-juce-for-dsp-and-ui-performance-tuning/

### Melatonin Inspector — UI Debugging

**What it is**: A JUCE module that gives you a Web Inspector / Figma-like tool for inspecting and live-editing JUCE component properties. Visualize component hierarchy, sizes, positions, and visibility in real-time.

**Why it matters**: JUCE UI debugging is otherwise a lot of guesswork — "why isn't this component showing up?" Instead of littering code with `DBG()` calls, pop open the inspector and visually see what's going on. Especially useful for the complex tracker grid and mixer layouts.

**How to use it**:
```cpp
#include "melatonin_inspector/melatonin_inspector.h"

// In your editor, toggle with a key command
std::unique_ptr<melatonin::Inspector> inspector;

void toggleInspector() {
    if (!inspector) {
        inspector = std::make_unique<melatonin::Inspector>(*this);
        inspector->onClose = [this]() { inspector.reset(); };
    }
    inspector->setVisible(true);
}
```

- GitHub: https://github.com/sudara/melatonin_inspector
- Includes FPS meter and component timing

### juce-toys — Visual Studio NatVis & Debugger Helpers

**What it is**: NatVis files for Visual Studio and LLDB scripts that make JUCE types readable in the debugger. Without these, inspecting a `juce::String` in VS shows raw pointer internals instead of the actual string value.

**What it provides**:
- NatVis for Visual Studio: readable display of `juce::String`, `juce::var`, `juce::ValueTree`, `juce::Array`, `juce::Rectangle`, and more
- LLDB scripts for Xcode/CLion with equivalent functionality
- ComponentDebugger, ValueTreeDebugger, BufferDebugger utility classes
- Lock-free inter-thread utilities (garbage collector, nonblocking call queue)
- Audio sparklines in the debugger (visualize audio buffers as waveforms in the IDE)

**Install the NatVis file on day one** — it's a tiny investment that pays off every single debug session.

- GitHub: https://github.com/jcredland/juce-toys

### Clang Sanitizers (ASan, TSan, UBSan)

Beyond RTSan, the standard Clang sanitizers are invaluable for a C++ project of this complexity:

- **AddressSanitizer (ASan)**: catches buffer overflows, use-after-free, double-free. Compile with `-fsanitize=address`. Essential for sample buffer manipulation code.
- **ThreadSanitizer (TSan)**: detects data races between threads. Compile with `-fsanitize=thread`. Critical for UI↔audio thread communication. Catches issues that only manifest as rare, unreproducible glitches.
- **UndefinedBehaviorSanitizer (UBSan)**: catches integer overflow, null pointer dereference, type punning violations. Compile with `-fsanitize=undefined`.

**Recommendation**: Run ASan and UBSan in Debug builds by default. Run TSan periodically (it has higher overhead). These are available in both Clang and GCC, and ASan/UBSan work in MSVC as well.

### Pamplejuce — Project Template Reference

**What it is**: A well-maintained JUCE plugin project template that bundles best practices: CMake setup, Catch2 testing, Melatonin Inspector, Perfetto integration, CI/CD with GitHub Actions, code signing, and pluginval validation.

**Why it matters**: Even though ChipForge is a standalone app (not a plugin), Pamplejuce's CMake structure and tooling integration are a great reference for setting up the project skeleton correctly from the start.

- GitHub: https://github.com/sudara/pamplejuce

### Built-in JUCE Debugging Features

JUCE itself provides several debugging aids to use from day one:

- `jassert()` — debug-only assertions (like standard `assert` but with JUCE integration). Use liberally.
- `DBG()` — debug logging macro. **Warning**: `DBG` allocates memory, so never use it in the audio callback in production. Fine for temporary debugging but remove before testing for glitches.
- `JUCE_ENABLE_REPAINT_DEBUGGING` — highlights component repaints in random colors so you can see what's being repainted and how often.
- `juce::LeakedObjectDetector` — JUCE automatically detects leaked objects in Debug builds. If you see "Leaked objects detected" on shutdown, fix it immediately.
- `juce::UnitTest` — built-in test framework for running tests within the JUCE environment.

### Debugging Strategy for Claude Code Sessions

When debugging audio issues in CC, follow this diagnostic order:

1. **Does it compile cleanly with no warnings?** Treat warnings as errors (`-Wall -Werror` or `/W4 /WX`).
2. **Does it pass sanitizers?** Run with ASan+UBSan first, then TSan.
3. **Is the audio callback taking too long?** Check with Perfetto traces.
4. **Is something blocking the audio thread?** RTSan catches this, or manually check for allocations/locks/DBG calls in the call chain from `processBlock`.
5. **Is the UI layout correct?** Use Melatonin Inspector rather than guessing.
6. **Can you see what's in the audio buffer?** Use audio sparklines or write a quick WAV dump to disk for offline inspection.

### Integration Plan

Add these tools as Git submodules in Phase 0:
```
libs/
├── JUCE/                        # Git submodule
├── nes_snd_emu/                 # Git submodule or vendored
├── melatonin_inspector/         # Git submodule (debug builds only)
└── melatonin_perfetto/          # Git submodule (opt-in via PERFETTO=1)
```

Install the juce-toys NatVis file into your Visual Studio user directory on first setup. The juce-toys module itself can be added as a submodule if the lock-free utilities are useful, but the NatVis file alone is the highest-value item.

---

## Token Optimization & Context Management

This project will grow large — potentially 50-100+ C++ files. Without active context management, Claude Code will burn through tokens re-reading files every session. These tools keep token consumption under control.

### lean-ctx — Token Compression (MCP + Shell Hook)

**What it is**: A hybrid tool that compresses CLI output (shell hook) and provides cached, compressed file reads (MCP server). Single Rust binary, zero dependencies.

**Why it matters for this project**: The biggest token drain is CC re-reading source files. lean-ctx's `ctx_read` tool caches files by hash — re-reads of unchanged files cost ~13 tokens instead of ~2000. Its `map` mode shows just the dependency graph + API signatures at ~5-15% of full token cost, which is perfect for CC understanding module interfaces without reading implementation.

**Setup on both machines**:
```bash
# Install
cargo install lean-ctx

# Set up shell hook (compresses git, cmake, cargo output)
lean-ctx init --global

# Add to Claude Code
claude mcp add lean-ctx lean-ctx
```

**Key features for this project**:
- `ctx_read file.cpp -m map` — shows imports, exports, and function signatures (~5-15% tokens)
- `ctx_read file.cpp -m signatures` — just function/class signatures (~10-20% tokens)
- Cached re-reads of unchanged files = ~13 tokens each
- Shell hook compresses `cmake`, `git`, and build output automatically
- `lean-ctx gain` — dashboard showing cumulative token savings

- GitHub: https://github.com/yvgude/lean-ctx

### codemap — Codebase Structure & Cross-Machine Handoff

**What it is**: A Go tool that generates instant architectural context from your codebase — file trees, dependency graphs, hub file detection — and provides a handoff artifact system for switching between machines or AI agents without losing context.

**Why it matters for this project**: This directly solves your two-machine problem. When you finish a session on your home machine, `codemap handoff .` saves a layered artifact containing: the stable project structure (prefix layer), plus what changed recently, which files are risky, and what to work on next (delta layer). On your work machine, CC reads the handoff and picks up exactly where you left off.

**Setup**:
```bash
# macOS/Linux
brew tap JordanCoin/tap && brew install codemap

# Windows
scoop bucket add codemap https://github.com/JordanCoin/scoop-codemap
scoop install codemap

# Set up in project
cd /path/to/ChipForge
codemap setup
```

**Key features for this project**:
- `codemap .` — fast tree/context view of the project (respects filters)
- `codemap --diff` — what changed vs main branch
- `codemap --deps .` — dependency flow between files (uses ast-grep)
- `codemap handoff .` — save a layered handoff artifact for session continuity
- `codemap handoff --latest .` — read the last saved handoff
- `codemap context` — universal JSON context for any AI tool
- Tracks agent history across sessions (which files were edited, by whom)
- MCP server mode: `codemap mcp`

- GitHub: https://github.com/JordanCoin/codemap

### RepoMapper — PageRank-Based Code Map (Alternative/Complement)

**What it is**: A Python tool based on Aider's repo map concept. Uses Tree-sitter to parse code, builds a graph of file dependencies, applies PageRank to rank files/symbols by importance, and generates a token-optimized code map that fits within a budget.

**Why it matters**: For a large C++ codebase, not all files are equally important. RepoMapper identifies the "hub" files — the ones most referenced by others — and generates a map that focuses on those. This is great for giving CC the architectural overview without reading everything.

**Usage**:
```bash
python repomap.py . --max-context-window 8192
python repomap.py . --mentioned-files TrackerEngine.cpp  # Prioritize specific files
```

Also available as an MCP server for direct integration with Claude Code.

- GitHub: https://github.com/pdavis68/RepoMapper

---

## Multi-Machine Workflow & Session Handoff

Since you'll be working on both home and work machines, here's the recommended workflow to maintain continuity without relying on local CC memory:

### The Handoff Protocol

**End of session (Machine A):**
1. Commit all work to git with a descriptive message
2. Run `codemap handoff .` to save a handoff artifact (gets committed with the repo)
3. Optionally update `PROGRESS.md` (see below) if the handoff artifact isn't enough
4. Push to remote

**Start of session (Machine B):**
1. Pull from remote
2. CC reads `CLAUDE.md` (the project spec — this document) automatically
3. CC reads the codemap handoff: `codemap handoff --latest .`
4. CC reads `PROGRESS.md` for human-written context
5. Resume work

### PROGRESS.md — Human-Readable Session Log

Keep a `PROGRESS.md` at the project root that gets updated at the end of each CC session. This is the simplest, most reliable handoff mechanism:

```markdown
# ChipForge Progress

## Current Phase: Phase 2 — Tracker Engine Core

### Last Session: 2026-04-01 (Home)
- Completed: PatternCell data structure, Pattern class with 64-step default
- In Progress: TrackerEngine playback — step advancement works but effect
  commands not yet implemented
- Known Issues:
  - NES synth pitch is off by ~2 cents on high notes (issue in register calc)
  - UI cursor doesn't scroll when reaching bottom of visible pattern
- Next Steps: Implement arpeggio (0xy) and volume slide (Axy) commands

### Previous Sessions:
- 2026-03-31 (Work): Completed NES2A03Synth wrapper, all 4 channel types working
- 2026-03-30 (Home): Phase 1 complete — keyboard mode plays notes through NES synth
```

**Ask CC to update this file** at the end of every session. It takes 30 seconds and saves 10 minutes of re-orientation on the next machine.

### Git Branch Strategy

For a solo project across two machines, keep it simple:
- `main` — stable, working builds only
- `dev` — daily work branch, push/pull between machines
- Feature branches only when experimenting with something risky

### What Goes in .codemap/ (Committed to Git)
```
.codemap/
├── config.json          # codemap settings (auto-generated by setup)
└── handoffs/            # layered handoff artifacts (auto-generated)
```

### What Stays Local (Not Committed)
- `~/.lean-ctx/` — lean-ctx stats and cache (per-machine)
- CC's local memory/session data
- Build artifacts (`build/`)

---

## Claude Code Working Guidelines

### Context Management
This is a large C++ project. To work effectively in Claude Code:

1. **Work on one module at a time** — don't try to hold the entire codebase in context. Focus on implementing one class/file, test it, then move on.
2. **Start with interfaces** — define header files first, then implement. This establishes contracts between modules before writing implementation code.
3. **Test incrementally** — after each module, verify it compiles and the app still launches. Audio bugs compound quickly.
4. **Use JUCE's built-in testing** — JUCE provides `UnitTest` classes. Use them for data model and engine logic.

### Build Commands
```bash
# Configure (first time)
cmake -B build -G "Visual Studio 17 2022"

# Build
cmake --build build --config Release

# Run
./build/Release/ChipForge.exe
```

### Key JUCE Patterns to Follow
- Inherit from `juce::AudioAppComponent` for the main audio pipeline
- Use `juce::AudioProcessorGraph` if complex routing is needed later
- Use `juce::Component` for all UI elements
- Use `juce::LookAndFeel_V4` as base for custom theming
- Use `juce::ValueTree` for observable state that bridges UI ↔ engine
- Use `juce::AbstractFifo` or `juce::SingleThreadedAbstractFifo` for lock-free audio communication
- Use `juce::dsp::ProcessorChain` for effect chains

### What Not To Do
- Don't use `new`/`delete` — use `std::unique_ptr`, `std::shared_ptr`, or JUCE's memory management
- Don't allocate in the audio callback — pre-allocate all buffers
- Don't use `std::mutex` in anything touched by the audio thread
- Don't try to build the entire app in one session — this is a phased project
- Don't over-abstract early — get things working, then refactor

---

## Success Criteria (MVP)

The MVP is complete when a user can:

1. Launch the application and see the SNES-themed tracker interface
2. Select the NES 2A03 synth for a track
3. Toggle keyboard mode and play notes — hearing NES sounds in real-time
4. Enter notes into the pattern grid using the keyboard
5. Play back a pattern and hear the sequenced notes
6. Create multiple patterns and arrange them into a song
7. Apply effects (reverb, delay, filter) to individual tracks
8. Adjust volume and panning per track in the mixer view
9. Load a WAV sample and use it as an instrument
10. Save the project and reload it
11. Export the song to a WAV file

If all 11 items work, ChipForge v0.1 is ready for testing.
