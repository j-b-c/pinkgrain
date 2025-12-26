# Changelog

All notable changes to PinkGrain will be documented in this file.

## [1.2.0] - 2025-12-26

### Added
- **Preset System**: Save and load presets via dropdown menu, stored in ~/Library/Application Support/PinkGrain/Presets/
- **Session Persistence**: Plugin automatically restores previous session state (loaded file and all parameters) on launch
- **Volume Meter**: Horizontal volume control with live stereo level display
- **Per-Note Grain Release**: Grains now release when their triggering MIDI note is released, not when all notes are released

### Changed
- Font changed to Helvetica for cleaner appearance
- Volume control redesigned as horizontal bar with integrated level meter
- Text color unified with primary pink (#FF1493)

### Fixed
- Grain envelope release now correctly responds to individual note-off events

## [1.1.0] - 2025-12-22

### Added
- **Zoomed Grain Window Display**: New display below the main waveform showing a zoomed-in view of the current grain window with animated grain dots
- **Click-and-Drag Position Control**: Click and drag on the main waveform display to set the playback position
- **VBlank Synchronization**: Display refresh now syncs to monitor refresh rate for smoother animations

### Changed
- Plugin window height increased from 500 to 600 pixels to accommodate the new zoomed display
- Grain dots moved from main waveform display to the new zoomed display for better visibility
- Grain dot positions now accurately reflect the actual file position being read
- Grain dots correctly move in reverse direction when reverse mode is enabled

### Fixed
- Grain visualization now properly shows the portion of the audio file being played
- Dot alpha correctly reflects envelope level (attack/release visualization)

## [1.0.0] - 2025-12-22

### Added
- Initial release
- Granular synthesis engine with up to 128 simultaneous grains
- MIDI input with pitch tracking (middle C = original pitch)
- Waveform display with grain window overlay
- Live output waveform display
- Parameters: Size, Density, Position, Pitch, Pan Spread, Spray, Attack, Release, Reverse, Pitch Random, Volume
- Support for WAV, AIFF, MP3, and FLAC audio files
- VST3 and AU plugin formats
- Dark theme with pink accents
