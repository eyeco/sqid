# Changelog

## [0.1.3] - 2026-06-09
### Modified
- renamed source > interface
- renamed sensor > source

### Added
- PID operator
- port names in COM/serial interface inspector dropdown
- added sink operator

### Removed
- legacy FW props

### Deprecated
- serialOut operator

### Fixed
- operators merging multiple input frames are now choosing most-recent timestamp from those inputs for the output frame
- autosave bug
