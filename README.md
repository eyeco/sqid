# sqid Visual Programming Environment

_TODO: general description_

## Projects

### sqid

Basic data processing of raw sensor data (as received via OSC) and sending of processed data (also via OSC) with optional graphical output.

#### Build

Only tested on windows, so far, using MSVS 2017.

Note: Support of RFCOMM (i.e. Bluetooth), Sensel, TUIO, MIDI, and gamepad sources are optional, if according dependencies are missing you can skip them by preprocessor switches. Use the respective defines `__RFCOMM_SUPPORT`, `__SENSEL_SUPPORT`, `__TUIO_SUPPORT`, `__MIDI_SUPPORT`, and `__GAMEPAD_SUPPORT` in [common.h](./src/common.h) accordingly.

Dependencies:
##### General (used across modules and applications)
- OpenCV (v 3.0 upwards, tested with 4.1.0, 64 bit) [[link](https://opencv.org/)]
- cxxopts (tested with version 3.2.0, 64 bit) [[link](https://github.com/jarro2783/cxxopts)]
- glew (tested with version 2.1.0, 64 bit) [[link](http://glew.sourceforge.net/)]
- glfw (tested with version 3.3, 64 bit) [[link](https://www.glfw.org/)]
- glm (tested with version 0.9.9.3) [[link](https://github.com/g-truc/glm)]
- Dear ImGui (tested with 1.91.1) [[link](https://github.com/ocornut/imgui)]
- nlohmann json (tested with version 3.5.0) [[link](https://github.com/nlohmann/json)]
- serial (tested with version 1.2.1) [[link](https://github.com/wjwwood/serial)]
- liblo (tested with version 0.31, 64 bit) [[link](http://liblo.sourceforge.net/)]
- KindDragon Visual Leak Detector (tested with version 2.5.1) [[link](https://github.com/KindDragon/vld)]

##### Core
- bluetooth serial port [[link](https://github.com/Agamnentzar/bluetooth-serial-port)]
- clipboardXX (tested with commit [#d404c39](https://github.com/Arian8j2/ClipboardXX/tree/d404c39)) [[link](https://github.com/Arian8j2/ClipboardXX)]
- FreeType (tested with version 2.10.1, 64 bit) [[link](https://freetype.org/)]
- dlfcn-win32 (tested with version 1.4.1, 64 bit) [[link](https://github.com/dlfcn-win32/dlfcn-win32)]
- FFTW (tested with version 3.3.8, 64 bit) [[link](https://www.fftw.org/)]
- MiniLZO (tested with version 2.10, 64 bit) [[link](http://www.oberhumer.com/opensource/lzo/#minilzo)]
- QuickLZ (tested with version 1.5.0, 64 bit) [[link](http://www.quicklz.com/)]
- bzip2 (tested with version 1.0.6, 64 bit) [[link](https://www.sourceware.org/bzip2/)]
- zStd (tested with version 1.4.0, 64 bit) [[link](https://facebook.github.io/zstd/)]
- zLib (tested with version 1.2.11, 64 bit) [[link](https://zlib.net/)]
- LZ4 (tested with version 1.8.3, 64 bit) [[link](https://lz4.github.io/lz4/)]
- libjpeg-turbo (tested with version 1.5.3, 64 bit) [[link](https://libjpeg-turbo.org/)]

##### Plugins
- ZeroMQ (tested with 4.3.2, 64 bit) [[link](https://zeromq.org/)]
- Eclipse Mosquitto MQTT (tested with 2.0.14, 64 bit) [[link](https://mosquitto.org/)]
- Peak System PCAN-Basic API (tested with version 4.5.4.508) [[link](https://www.peak-system.com/Development.526.0.html)]
- TUIO 2.0 [[link](https://github.com/mkalten/TUIO20_CPP)]
- NvGamepad (tested with 1.0, 64 bit) [[link](https://developer.nvidia.com/cross-platform-gamepad-api)]
	- Microsoft DirectX End-User Runtime [[link](https://www.microsoft.com/en-us/download/details.aspx?id=35)]
- RtAudio (tested with 5.1.0, 64 bit) [[link](https://www.music.mcgill.ca/~gary/rtaudio/)]
- RtMidi (tested with 4.0.0, 64 bit) [[link](https://github.com/thestk/rtmidi)]
- LeapSDK (tested with 4.0.0, 64 bit) [[link](https://docs.ultraleap.com/api-reference/tracking-api/index.html#)]
- Myo SDK (tested with 0.9.0, 64 bit) [[link](https://support.getmyo.com/hc/en-us/articles/360018409792-Myo-Connect-SDK-and-firmware-downloads)]
- senselLib (tested with 0.8.3, 64 bit) [[link](https://github.com/sensel/sensel-api)]
- Kinect for Windows SDK 1 (tested with 1.8, 64 bit) [[link](https://www.microsoft.com/en-us/download/details.aspx?id=40278)]
- librealsense2 (tested with version 2.36.0.2034, 64 bit) [[link](https://github.com/IntelRealSense/librealsense)]
- OptiTrack Camera SDK (tested with 2.2.0) [[link](https://optitrack.com/software/camera-sdk/)]
- OptiTrack NatNet SDK (tested with 2.10) [[link](https://optitrack.com/software/natnet-sdk/)]

#### TODO
- implement a more reasonable serial communication protocol
- implement Firmata (maybe into core)
- implement bluetooth serial in a solid way (and maybe put it to a plugin)
- move core out of modules
- add all remaining ops and framedrawers
- catch exception when parsing config.json (or put proper error message)
- fix CAN and OSC -> implement as *sources*
- clean up comments/includes/etc.
- implement BlobFrame
- implement CompressedSampleFrame
- extend PointCloud processing
- fix issues
	- something seems to be wrong with UI JSON (esp. when using plugins like RealSense, the layout is messed up at startup)
	- figure out what the problem with ImGui TreeNodes is (cannot be opened anymore after having the application running for some time)
	- investigate issue with Thomas' PC which is apparently unable to send OSC packages
	- apparently, when OSC sending is enabled already at startup, a lot of data is buffered during GL window initialization, which is quickly exceeding the OSCProxy's buffer size. Consider starting to send only when everything is initialized and running properly.
	- fix saving to INI: multiple nodes of same type all write to same JSON node. Create each node with GUID, then save scene configuration. maybe do this along rehaul for creating wiring UI, which would need an overall scene-savefile anyways.
	- print IP adapter information at startup
- implement trajectory tracking for BlobTracker
- implement multi-pin input/output
- make little demo for smart phone
- data visualization overhaul
	- enable switching visualization between maps (min/max/snapshot/etc.)
	- display history of array as 2D plot, just like e.g. spectrogram
	- do line-visualization of 0-history as bar
- additional sources
	- Bluetooth LE (apparently not adequate for continuously-sending devices? have a look [here](https://docs.microsoft.com/en-us/windows/uwp/devices-sensors/bluetooth-low-energy-overview))
	- named pipe
	- legacy serial protcol for backwards compatibility (e.g. flextiles)
	- Sensel via Bluetooth
	- finish generator source
	- CAN source
	- index picker: pick value at specific x/y/z from matrix/SF
- additional processors
	- fft
	- history => 1D map
	- 1D + history => 2D map
	- temporal resampling
- add inter-/extrapolation to temporal resampling (Op currently only duplicates last frame)
- finish up saving/loading to/from INI file
- complementary for bluetooth sources
	- signal strength (not guaranteed this is supported at all -- look into Windows 10 support of RSSI (resource signal strength indicator))
	- figure out if class 1, 2, or 3 (bandwidth/signal strength?)
- optimizatons
	- profile and avoid unneccessary copies at in/out pins
	- only push to output pins that are actually connected
- provide posibilities to (and options of how to) sync multiple inputs
- configuration parameters of processing nodes in configuration JSON file
- handle Op properties that are not found during loading from JSON file (current behaviour is to return 0 which causes a crash in many cases)
- use SampleFrame's timing data for time corrected processing
- implement plugin system and move optional Op packages out of core codebase
- add some safety-checks for GUIDs in opFactory (throw if empty GUIDs are provided or GUIDs are already used by other Ops)
- cut OSC send frequency, in most cases there's probably no need to send every single processed frame to a frontend
- extend senders
	- sink for OSC via TCP for sending larger packets: switch to different OSC library, e.g. [liblo](http://opensoundcontrol.org/implementation/liblo-lightweight-osc-api) or [WOscLib](http://opensoundcontrol.org/implementation/wosclib) for C++, [Bespoke OSC](http://opensoundcontrol.org/implementation/bespoke-osc-net-2-0) (behold incomplete type support) for C#
	- sink for Bluetooth (RFCOMM)
- expose available serial and bluetooth devices in GUI and make selectable
- file playback
	- consider adding timeline slider (or some other possibility to jump on timeline)
	- time scaler (for timelapse/slow-motion)
- gesture recognition
	- implement 1D/2D swipe detection processor
	- implement 1D/2D blob-tracking processor
	- implement Wobbrock's [$1 unistroke recognizer](http://depts.washington.edu/madlab/proj/dollar/index.html)
	- include Nick Gillian's [GRT](http://www.nickgillian.com/wiki/pmwiki.php/GRT/GestureRecognitionToolkit) for ML powered recognition
	- implement [$Q](https://dl.acm.org/citation.cfm?id=3229465) for stroke recognition (also have a look at [$P](https://dl.acm.org/citation.cfm?id=2388732), [1¢](https://dl.acm.org/citation.cfm?id=2331074), and the whole [$ family](http://depts.washington.edu/acelab/proj/dollar/impact.html))
- consider live-interface to MATLAB/Simulink
- MQTT
	- move peers' (currently blocking) connection attempt away from main thread
	- implement TLS connection
- CAN
	- add different methods for sending data (instead of just sending single-values in the first byte which is adequate for the current use case, but quite limited)
	- add bitrate-presets (with reasonable pre-configured nominal and fast-data properties; see the pre-configured settings in [PCanView.exe](https://www.peak-system.com/PCAN-View.242.0.html) for a reference)
	- provide possibility to operate several canIn's next to each other, listening to different msgIDs (instead of reading in one Op and rejecting all packages that do not match Op's msgID filter)
- move task-/project-specialized Ops somewhere else (e.g., providing plugin system may help a lot), e.g.:
	- MaxPooling
	- SensorSyncMerge
- some data sinks and sources (e.g., CAN, MQTT, OSC) are implmented quite badly, since one Op is usually associated with a specific port/device/... have to find a way to send multiple messages to same target as well as receiving different messages from same source and filter/relay within processing graph

### oscConsole

Console program used to send test OSC messages.

#### Build

Dependencies:
- cxxopts (tested with version 3.2.0, 64 bit) [[link](https://github.com/jarro2783/cxxopts)]
- liblo (tested with version 0.30, 64 bit) [[link](http://liblo.sourceforge.net/)]

### oscListener

Console program for OSC monitoring: prints OSC data to console window.

#### Build

Dependencies:
- cxxopts (tested with version 3.2.0, 64 bit) [[link](https://github.com/jarro2783/cxxopts)]
- liblo (tested with version 0.30, 64 bit) [[link](http://liblo.sourceforge.net/)]

### TCP2OSC

Workaround C# application for converting TCP raw data stream to OSC packages, as UDP implementation on NodeMCU seemed to be faulty.

#### Build

Dependencies:
- OSCsharp [[link](https://github.com/valyard/OSCsharp)]

## Author

Roland Aigner [[link](https://www.rolandaigner.com)]

## License

Copyright (C) 2024 eyeco

sqid Visual Programming Environment is licensed under the GPL3 License. See LICENSE file in the package root for license information.
