# sqid Visual Programming Environment

_TODO: general description_

A user documentation can be found [here](./doc/main.md).

## Projects

### sqid

Basic data processing of raw sensor data (as received via OSC and numerous other sources) and sending of processed data (also via OSC) with optional graphical output.

#### Build

Only tested on windows, so far, using MSVS 2017.

Note: Support of 2D FFT (via FFTW library), compression, and RFCOMM (i.e. Bluetooth) are optional; you can deactivate them by preprocessor switches. Use the respective defines `__FFTW_SUPPORT`, `__COMPRESSION_SUPPORT`, and `__RFCOMM_SUPPORT` in [common.h](./modules/sqidCore/include/common.h) accordingly. You can also build a head-less version without GUI (not just deactivating but purging all GL dependencies, e.g., for CL-only operating systems) by removing `__SUPPORT_GUI`. Note however that plugins binaries must match all of these configurations.

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

##### Plugins (included)
- RtAudio (tested with 5.1.0, 64 bit) [[link](https://www.music.mcgill.ca/~gary/rtaudio/)]
- RtMidi (tested with 4.0.0, 64 bit) [[link](https://github.com/thestk/rtmidi)]
- Eclipse Mosquitto MQTT (tested with 2.0.14, 64 bit) [[link](https://mosquitto.org/)]
- TUIO 2.0 [[link](https://github.com/mkalten/TUIO20_CPP)]
- ZeroMQ (tested with 4.3.2, 64 bit) [[link](https://zeromq.org/)]

##### Plugins (external)
- Peak System PCAN-Basic API (tested with version 4.5.4.508) [[link](https://www.peak-system.com/Development.526.0.html)]
- NvGamepad (tested with 1.0, 64 bit) [[link](https://developer.nvidia.com/cross-platform-gamepad-api)]
	- Microsoft DirectX End-User Runtime [[link](https://www.microsoft.com/en-us/download/details.aspx?id=35)]
- Kinect for Windows SDK 1 (tested with 1.8, 64 bit) [[link](https://www.microsoft.com/en-us/download/details.aspx?id=40278)]
- LeapSDK (tested with 4.0.0, 64 bit) [[link](https://docs.ultraleap.com/api-reference/tracking-api/index.html#)]
- Myo SDK (tested with 0.9.0, 64 bit) [[link](https://support.getmyo.com/hc/en-us/articles/360018409792-Myo-Connect-SDK-and-firmware-downloads)]
- OptiTrack NatNet SDK (tested with 2.10) [[link](https://optitrack.com/software/natnet-sdk/)]
- OptiTrack Camera SDK (tested with 2.2.0) [[link](https://optitrack.com/software/camera-sdk/)]
- librealsense2 (tested with version 2.36.0.2034, 64 bit) [[link](https://github.com/IntelRealSense/librealsense)]
- senselLib (tested with 0.8.3, 64 bit) [[link](https://github.com/sensel/sensel-api)]

#### TODO
- implement a more reasonable (less historically burdened) serial communication protocol
- implement Firmata (maybe into core)
- implement Kinect for XBox One plugin
- implement Azure Kinect plugin
- implement OpenNI plugin
- implement NITE plugin
- implement bluetooth serial in a solid way (and maybe put it to a plugin)
- move core out of modules
- catch exception when parsing config.json (or put proper error message)
- clean up comments/includes/etc.
- implement BlobFrame and CompressedSampleFrame
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
- provide plugins/addons for 3rd-party connectivity
  - MATLAB
  - Rhino3D/Gh
  - vvvv
  - TouchDesigner
- provide some demos/templates
  - Unity3D
  - Processing
  - Pd
  - Android smart phone
- data visualization overhaul
	- enable switching visualization between maps (min/max/snapshot/etc.)
	- display history of array as 2D plot, just like e.g. spectrogram
	- do line-visualization of 0-history as bar
- additional interfaces
	- Bluetooth LE (apparently not adequate for continuously-sending devices? have a look [here](https://docs.microsoft.com/en-us/windows/uwp/devices-sensors/bluetooth-low-energy-overview))
	- named pipe
	- finish generator source
	- index picker: pick value at specific x/y/z from matrix/SF
- additional processors
	- fft
	- history => 1D map
	- 1D + history => 2D map
	- temporal resampling
- add inter-/extrapolation to temporal resampling (Op currently only duplicates last frame)
- finish up saving/loading to/from INI file
- complementary for bluetooth interfaces
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
- move task-/project-specialized Ops somewhere else (e.g., providing plugin system may help a lot), e.g.:
	- MaxPooling
	- SensorSyncMerge
- some data sinks and sources (e.g., CAN, MQTT) are implemented quite badly, since one Op is usually associated with a specific port/device/... have to find a way to send multiple messages to same target as well as receiving different messages from same source and filter/relay within processing graph

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

## Author

Roland Aigner [[link](https://www.rolandaigner.com)]

## License

Copyright (C) 2024 eyeco

sqid Visual Programming Environment is licensed under the GPL3 License. See LICENSE file in the package root for license information.
