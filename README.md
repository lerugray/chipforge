# ChipForge

A chiptune tracker DAW for Windows, inspired by the Polyend Tracker and Renoise. Built for composing 8-bit and 16-bit era video game music with a SNES-inspired retro UI.

![Phase 0 screenshot — themed window with SNES color palette](docs/screenshot-phase0.png)

> **Status: Phase 0 — Project skeleton.** The app launches, plays a test tone, and displays the themed UI. Active development toward Phase 1 (NES sound chip integration).

---

## What is ChipForge?

ChipForge combines the step-entry, pattern-centric workflow of hardware trackers with the power of a desktop DAW:

- **Chip-accurate NES sound emulation** (2A03 APU — pulse, triangle, noise, DPCM)
- **Tracker-style pattern editor** — enter notes by row, compose in patterns, arrange into songs
- **Built-in effects** — reverb, delay, filter, chorus, distortion, bit crusher
- **SNES-era aesthetic** — deep navy-purple palette, pixel fonts, clean retro UI
- **Sample engine** — load WAV files, pitch them to notes, sequence them in patterns
- **Export** — render to WAV, per-track stems, MIDI export

Designed for composers who want the feel of classic hardware trackers (FamiTracker, Renoise) with the convenience of a modern desktop application.

---

## Technology Stack

| Component | Technology |
|-----------|-----------|
| Framework | [JUCE 8](https://github.com/juce-framework/JUCE) (C++20) |
| Audio I/O | WASAPI / ASIO via JUCE |
| GUI | JUCE Direct2D renderer |
| NES emulation | [Nes_Snd_Emu](https://github.com/jamesathey/Nes_Snd_Emu) |
| Build system | CMake 3.22+ |
| Compiler | MSVC (VS2022), C++20 |
| UI debugging | [Melatonin Inspector](https://github.com/sudara/melatonin_inspector) |
| Audio profiling | [Melatonin Perfetto](https://github.com/sudara/melatonin_perfetto) (optional) |

---

## Building from Source

### Prerequisites

- Windows 10/11
- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **"Desktop development with C++"** workload
- [CMake 3.22+](https://cmake.org/download/) — install and add to PATH
- Git

### Clone

```bash
git clone --recurse-submodules https://github.com/YOUR_USERNAME/chipforge.git
cd chipforge
```

> The `--recurse-submodules` flag is important — it pulls JUCE and the other libraries automatically.

### Build

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --parallel 1
```

> **Note on `--parallel 1`:** JUCE's large headers can exhaust MSVC's compiler heap when building multiple files simultaneously. Single-threaded compilation is slower but reliable. This will be improved in a future build configuration.

### Run

```bash
./build/ChipForge_artefacts/Debug/ChipForge.exe
```

---

## Development Status

| Phase | Description | Status |
|-------|-------------|--------|
| 0 | Project skeleton — app launches, themed UI, test tone | ✅ Done |
| 1 | NES 2A03 synth integration — hear real chip sounds | 🔜 Next |
| 2 | Tracker engine — enter notes, play back patterns | ⏳ Planned |
| 3 | Effects & mixer — per-track effects, faders, VU meters | ⏳ Planned |
| 4 | Sample engine — load and sequence WAV samples | ⏳ Planned |
| 5 | Arrangement view — arrange patterns into full songs | ⏳ Planned |
| 6 | Project save/load & WAV export | ⏳ Planned |
| 7 | MIDI input | ⏳ Planned |
| 8 | Polish & instrument editor | ⏳ Planned |

---

## Keyboard Shortcuts (Phase 0)

| Key | Action |
|-----|--------|
| `Ctrl+Shift+I` | Toggle Melatonin Inspector (debug builds only) |

---

## Project Structure

```
src/
├── App/          # Application entry point and main window
├── Engine/       # Audio engine, transport state, tracker playback
├── Synths/       # NES 2A03 synth wrapper, sample player
├── Effects/      # Effect chain, reverb/delay/filter/etc.
├── Data/         # Pattern, Song, Instrument, Project data structures
├── MIDI/         # MIDI input and export
├── Export/       # WAV and stem export
└── UI/           # All UI components and custom LookAndFeel
libs/
├── JUCE/                  # JUCE framework (submodule)
├── melatonin_inspector/   # UI debug inspector (submodule)
├── melatonin_perfetto/    # Audio profiling (submodule)
└── nes_snd_emu/           # NES APU emulator (submodule)
```

---

## License

TBD — considering MIT or GPL depending on the NES emulator license requirements.

---

## Inspiration & References

- [Polyend Tracker](https://polyend.com/tracker/) — workflow and UX inspiration
- [Renoise](https://www.renoise.com/) — desktop tracker reference
- [FamiTracker](http://famitracker.com/) — NES tracker, effect command vocabulary
- [Furnace](https://github.com/tildearrow/furnace) — open source multi-system chiptune tracker
- [FamiStudio](https://famistudio.org/) — modern NES music editor
