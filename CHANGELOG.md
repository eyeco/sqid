# Changelog

## [0.1.4] - 2026-07-01
### Modified
- renamed 'source' > 'interface'
- renamed 'sensor' > 'source'
- extended _PID_ controller operator
- interpreting 'c' OSC type as plain int now, removed normalization to [0 1] range
- allowing multiple OSC arguments for native types now -- dynamically building SampleFrame from parsed arguments of supported types
- changed default configuration of _sum_ operator -- now adding up all components by default, instead of none
- changed output of _time_ and _timestamp_ operators from 1x1 to 5x1 SampleFrame, now splitting seconds into d:h:m:s:ms (from left to right) to circumvent float32 limitation when ts values go beyond ±2<sup>24</sup>

### Added
- _PID_ operator
- now using port names in COM/serial interface inspector dropdown instead of plain COM port descriptors
- added _sink_ operator along with sending functionality of interfaces (previously 'sources')
- added support of OSC native data types (standard as per definition + liblo extended types), apart from blob and symbol
- fps counting added to sink and source
- changed _time_ operator to support output of _relative_ time (since reset or application start), _application_ time (since application start), _UNIX epoch_ time (since UNIX epoch 1970-01-01 00:00:00 UTC), and _boot_ (since system boot time); _UNIX epoch_ mode is furthermore supporting local timezones but defaults to UTC

### Removed
- legacy FW props

### Deprecated
- _serialOut_ operator

### Fixed
- operators merging multiple input frames are now choosing most-recent timestamp from those inputs for the output frame
- fixed autosave bug
