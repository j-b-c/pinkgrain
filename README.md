# PinkGrain

A granular synthesizer plugin built with JUCE. Supports VST3 and AU formats on macOS.

## Features

- **Granular Synthesis**: Load any audio file and transform it into evolving textures
- **MIDI Control**: Pitch follows MIDI notes (middle C plays at original pitch)
- **Real-time Visualization**: Waveform display with grain window overlay
- **Zoomed Grain Window**: Dedicated display showing zoomed-in grain window with animated grain dots
- **Click-and-Drag Position**: Click and drag on the waveform to set playback position
- **Live Output Display**: See the output waveform in real-time
- **VBlank Sync**: Display updates synchronized to monitor refresh rate
- **Preset System**: Save and load presets with automatic storage
- **Session Persistence**: Automatically restores previous session on launch
- **Per-Note Release**: Grains release individually when their MIDI note is released

### Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| Size | 10ms - 30s | Duration of each grain |
| Density | 1 - 100 g/s | Number of grains spawned per second |
| Position | 0 - 100% | Playback position in the loaded file |
| Pitch | -24 to +24 st | Pitch offset in semitones |
| Pan Spread | 0 - 100% | Stereo spread of grains |
| Spray | 0 - 100% | Randomization of grain start position |
| Attack | 0 - 100ms | Grain envelope attack time |
| Release | 0 - 500ms | Grain envelope release time |
| Reverse | On/Off | Play grains in reverse |
| Pitch Rnd | 0 - 24 st | Random pitch variation |
| Volume | 0 - 100% | Master output volume |

### Supported Audio Formats

- WAV
- AIFF
- MP3
- FLAC

## Building

### Requirements

- macOS 10.13+
- Xcode with command line tools
- CMake 3.22+

### Build Commands

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

### Output

Plugins are built to:
- `build/PinkGrain_artefacts/Release/AU/PinkGrain.component`
- `build/PinkGrain_artefacts/Release/VST3/PinkGrain.vst3`

With `COPY_PLUGIN_AFTER_BUILD` enabled, plugins are automatically installed to:
- `~/Library/Audio/Plug-Ins/Components/` (AU)
- `~/Library/Audio/Plug-Ins/VST3/` (VST3)

## Project Structure

```
synth/
├── CMakeLists.txt
├── README.md
├── CHANGELOG.md
└── Source/
    ├── PluginProcessor.h/cpp    # Audio processing, MIDI, presets, session
    ├── PluginEditor.h/cpp       # Main UI
    ├── Grain.h/cpp              # Individual grain with per-note tracking
    ├── GrainEngine.h/cpp        # Grain pool and spawning logic
    ├── AudioFileLoader.h/cpp    # Audio file loading and thumbnails
    └── UI/
        ├── LookAndFeel.h/cpp           # Pink/black theme
        ├── CustomDial.h/cpp            # Rotary dial component
        ├── WaveformDisplay.h/cpp       # File waveform with grain window
        ├── ZoomedWaveformDisplay.h/cpp # Zoomed grain window with dots
        ├── LiveWaveformDisplay.h/cpp   # Real-time output waveform
        └── VolumeControl.h/cpp         # Volume slider with level meter
```

## UI Theme

- Background: #0D0D0D (near black)
- Primary: #FF1493 (deep pink)
- Secondary: #FF69B4 (hot pink)
- Text: #FF1493 (deep pink)
- Font: Helvetica

## License

MIT
