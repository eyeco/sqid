# Changelog

## [0.1.4] - 2026-06-12
### Modified
- renamed 'source' > 'interface'
- renamed 'sensor' > 'source'
- extended _PID_ controller operator
- interpreting 'c' OSC type as plain int now, removed normalization to [0 1] range
- allowing multiple OSC arguments for native types now -- dynamically building SampleFrame from parsed arguments of supported types
- changed default configuration of _sum_ operator -- now adding up all components by default, instead of none

### Added
- _PID_ operator
- now using port names in COM/serial interface inspector dropdown instead of plain COM port descriptors
- added _sink_ operator along with sending functionality of interfaces (previously 'sources')
- added support of OSC native data types (standard as per definition + liblo extended types), apart from blob and symbol
- fps counting added to sink and source

### Removed
- legacy FW props

### Deprecated
- _serialOut_ operator

### Fixed
- operators merging multiple input frames are now choosing most-recent timestamp from those inputs for the output frame
- fixed autosave bug
