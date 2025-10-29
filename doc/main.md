# User Documentation

## General

Some features are selectively built into the main module, according to pre-processor switches. Refer to the console output to learn about included features e.g.
`
version 0.1.2
Release build for AMD64 with C++ language standard v201704
built with MSVC v1916(191627045) at Wed Jul  3 14:13:39 2024 with:
  GUI support                  YES
  Bluetooth Classic support    YES
  FFTW support                 YES
  Compression support          NO
`

Addon modules (such as support for MQTT, Kinect, Myo, etc.) are built as separate dll modules and dynamically loaded by the core module at startup (if configured to be loaded in the [config.json](../config.json) file). More details about how to do this will follow.

For replicating the serial communication from an ESP via USB, with the [firmware code](../firmware/) included in this package, find the USB to UART Bridge Virtual COM Port (VCP) drivers [here](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads).

## User guide

### How to run

Usage: `sqid [-g | --gui] [-h | --help] [sceneName]`

**Note: Make sure the working directory is the one containing the folders `resources` and `shaders`, since those will be sought at `./`, i.e., run the program from this directory.**

| Option            | Description                                       |
|-------------------|---------------------------------------------------|
| `-g, --gui`   | run with GUI (optional)                           |
| `-h, --help`  | print help (optional)                             |
| sceneName         | name of scene, without file extension (optional, defaults to 'scene')  |

Example: `sqid -g myScene` will run sqıd with GUI, loading the scene configuration from `myScene.json`, the UI layout from `myScene.ui.json`, and the window position/size, as well as preferences (auto-save, view settings) from `myScene.ini`. If the scene file does not yet exisit, it will be created. Scenes can be placed in subdirectory; in that case specify the absolute or relative path, e.g., `myDir/myScene`. **Note that directories must exist, they will not be created!** Since all required information is contained in the respective .json, .ui.json, and .ini files, scenes can be easily copied or moved just by copying or moving these three.

### Auto-save
Turn on/off the auto-save feature using the menu using 'File' > 'Auto-save scene'. With the feature turned on, changes on the scene and configuration will automatically be saved when the application is closed. **Note there is no automatic saving apart from that, e.g., during patching or configuration.**

### Mouse control

![selection](./img/left-select.gif)

Select operators by clicking left. Select multiple by holding Shift while clicking. Use selection window by clicking to canvas and dragging to select multiple. Again, hold Shift for additive selection.

![zoom](./img/mid-zoom.gif)

Zoom in and out using the mouse wheel, with the cursor as a pivot.

![move](./img/right-drag.gif)

Move the canvas by dragging with the right mouse button.

### Keyboard control

| key               | function                      |
| ----------------- | ----------------------------- |
| `Ctrl + q`    | quit (without saving, unless autosave is on) |
| `Ctrl + s`    | save scene (scene file name and path as specified at startup) |
| `Ctrl + c`    | copy selected sub-graph to clipboard |
| `Ctrl + v`    | paste from clipboard          |
| `Space`       | show/hide scene graph (to clear view when visualizer is maximized, see below) |
| `F1`          | open finder                   |
| `Esc`         | close finder                  |
| `Delete`      | delete selection              |

### Interface

![name-collapse](./img/name-and-collapse.gif)

Operators consist of a name, a visualizer (live-preview of the data), input pins and/or output pins. Red color on an output pin signals activity, i.e., frames being pushed downstream. Names specify to the operator type by default, but can be overridden to custom string using the `name` field in the inspector. Nodes can be collapsed to save space (and performance for visualizer drawing) and restored using the `-`/`+` toggle button on the left edge. 

![maximize](./img/maximize.gif)

Moreover, for detailed inspection of specific operator data, a visualizer can be *maximized* by clicking the triangular button on the upper right corner of an operator. This will render the respective visualizer scaled up to fill the entire background of the scene. To get an unblocked view, the UI's visibility can be toggled using the `space` key.

![tooltip](./img/tooltip.png)

Hovering the mouse over an output pin displays a tool-tip overlay, giving the pin name, followed by the number of queued frames, the data type, and type-specific information. E.g., the output pin "out" of the video capture operator in the example below shows holds 1 frame of type `sf` (generic SampleFrame), of dimensions 640x480x3. Timestamp is 6294275, containing values of range [0 1] (min/max values in current frame).

![tooltip](./img/deactivate.gif)

Operators can be deactivated by unchecking the `enabled` checkbox in the Inspector, which is visualized by darkened nodes. This entirely stops the processing and therefore the output at the pins (blue: no frames are getting pushed). Operators can also be deactivated by code, in case there is an unresolvable error in the processing step, e.g., when faulty or incompatible frames are fed as input. In this case, the error must be resolved before the operators can be enabled again. Note the following example, where frames of different sizes (4x4 vs. 5x4) are fed to multiplication and subtraction operators, causing errors in both, sending them to disabled state.

![ctrl-click-valueslider](./img/ctrl-click-valueslider.gif)

Value sliders can be set to exact values by left-clicking them while holding down Ctrl. This is also helpful to enter values beyond the slider ranges. The inspector, menu, and context menu UI is implemented using [Dear ImGui](https://github.com/ocornut/imgui), further documentation on details can be found there. 

### Visualizers

![visualizers](./img/visualizers.gif)

Operators provide one or more visualizers to inspect live data. Most operators include the basic three types
- map: color map of current frame
- lines (x): line plot of current frame
- lines (t): temporal line plot of *n* previous frames
- map (t): temporal color map of *n* previous frames

Visualizers can be selected in the inspector by expanding the "drawing" group. Depending on type, additional configuration may be provided. E.g., the "lines" types allows for configuration of the y-axis zero position and scaling.

### Creating and deleting operators

![create-menu](./img/create-menu.gif)

Create operators using the context menu: right-click and select an operator type. Configure parameters in using the Inspector. We demonstrate this with a perlin noise generator here, which can be found in 'gnr8' > 'perlin'. We set frame size to 4x4x3.

![create-finder](./img/create-finder.gif)

Create operators using the finder: hit `F1` and start typing the name. Select the operator type in the list that appears and hit `Return`.

![delete](./img/delete.gif)

Delete one or multiple selected operators by hitting the `Delete` key.

![copy-paste](./img/copy-paste.gif)

Copy and paste one or multiple selected operators using the menu ('Edit' > 'Copy', 'Edit' > 'Paste') or the keyboard shortcuts `Ctrl+c` and `Ctrl+v`. The copy process clones all operators including their configuration and the subgraph's internal patching.

### Patching

![patch](./img/patch.gif)

Connect connect operators' outputs to inputs by dragging output pins to compatible (green) input pins (left-click). Output pins can be connected to multiple inputs, but not the other way around.

![repatch](./img/repatch.gif)

Connections can be cleared by left-clicking connected input pins. If an output is connected to an already connected input pin, the established connection will be replaced.

## Tutorial: how to set up to retrieve data from an MCU via serial port

This is a walkthrough for the task of setting up a very simple input that comes from an ESP that is connected via serial port, just to get you started with the very basics. The ESP is running the firmware that is also included in this package in the folder [ESP_firmware](../firmware/).

### Step 1: set up COM input
First, we create a "source" node, which is not an operator, but an entity that is required to retrieve arbitrary data from devices that can send arbitrary data. Since we cannot know how many sensors an MCU is sampling, and moreover, how many sub-devices it may be operating, we introduced the concept of "Device IDs" and "Sensor IDs" to distinguish. In the firmware code you can see that six ADCs are sampled (`multiSample()`) and the results are stored into a float array, which is copied into the frame stucture (via `frame.setData()`) and then sent via serial port (`sender.send()`).

`
const int DEVICE_ID = 0;
const int SENSOR_ID = 0;

const int ANALOG_IN_COUNT = 6;

//...

  float f[ANALOG_IN_COUNT];
  for(int i = 0; i < ANALOG_IN_COUNT; i++)
    f[i] = multiSample(i, 3);

  if(!frame.setData(reinterpret_cast<const uint8_t*>(f))){
    //...
  } else {
    sender.send();
  }
`

Note that the frame was earlier configured as float array and associated with the sender instance during setup (`sender.init()`):

`
SampleFrame frame( DEVICE_ID, SENSOR_ID, SampleFrame::L_ARRAY, SampleFrame::DT_FLOAT, ANALOG_IN_COUNT );
SenderSerial sender;

//...
void setup() {

  //...
  if( !sender.init( &frame ) )
    Serial.println( "failed to init pointSender" );
  //...
}
`

This means that even tough we sampled six ADCs in total, we treat the data as a single 6-value array to send it via serial in a more compact way. For identification within sqıd, we defined Device ID as 0 and Sensor ID also as 0.

Note that all of these are implementation details. All the user really has to know is that there is data coming in which is associated with a certain combination of device ID and sensor ID, the rest is done within sqıd.

![com-part1](./img/tutorial/com-part1.gif)

We create a source node for serial ('sources' > 'COM' in the context menu, alternatively open the finder with F1 and type "com" to bring it up). We select it, and configure using the inspector: we select the COM port from the dropdown list (since we just plugged the MCU, it does not yet show up, so we refresh the list by hitting "rescan"). We may modify baud rate and number of maximum queued packages, then press "open". We can see in the inspector that data is coming in ("started" and "synced") and that there are packages coming from one senders. By expanding the group, we can see device IDs (dID) and sensor IDs (sID) of queued frames. 

Next, we create an operator of the generic type *sensor*, which is used to selectively grab frames from sources. In the inspector, we associate it with input type 'COM', set it to receive from port 8, and specify device ID and sensor ID of the frame we're interested in (i.e., 0 for both). We can see in the node visualizer that data is coming in, as it changes from black to a heatmap visualization. 

![com-part2](./img/tutorial/com-part2.gif)

Since in this example we attached four sensors, but the firmware is sampling six ADCs, the last two floats of the array are not in use and stay at 0. Hence, we are only interested int the sub-array of elements [0...3] and want to operate with a 4x1 frame from here on. We create a *crop* operator ('util' > 'crop') and set 'right' to 0.75, to crop to the leftmost 75% (technically 2/3 would be correct, but the width is floored to the 4 anyways). When we hover the mouse cursor over the output pin, we can see that the output is of size '4x1'.

We want a signal that is 0 when there is no activity, and reaches up to 1 for full saturation. Since we used resistive sensors in a voltage divider, the voltage drops when they are actuated, so signal is actually upside-down: high voltage at inactivity, low voltage when they're actuated. The most simple way to fix this is to create an *invert* operator ('math' > 'invert'), which transform each frame element by x'=(1-x). For better clarity, we switch the visualizers to temporal line drawing ('line (t)') to get a temporal tend of the signal.

![com-part3](./img/tutorial/com-part3.gif)

By inspecting the min/max markers in the visualizer, we see the inverted signal values are ~0.21 at rest and ~0.87 at saturation; also, the array sensors' min and max values differ slightly. For a quick'n'dirty calibration want to map them to ranges of about [0 1]. The *autoNormalize* operator ('math' > 'transform' > 'autoNormalize') is meant to facilitate this. To better utilize the line visualizer's displayed range, we offset drawing by -1 and scale by 2, thus setting the drawing range from the default [-1 1] to [0 1].

![com-part4](./img/tutorial/com-part4.gif)

it can use a "learning phase" to discover minima and maxima of either (i) individal frame elements or (ii) of the entire frame. The elements are transformed by x'=(x-min)/(max-min). We want each element to be mapped individually, so we leave the radio button at "individually". We check the "learn" checkbox to activate learning phase and actuate each of the sensors to maximum, one after the other. After this calibration phase, minima and maxima were recorded for each frame element. To no longer have them modified during operation, we uncheck the "learn" checkbox, so they are set. 

We see in the autoNormalize node, that the range of [0 1] is reasonably exhausted, giving us now a value mapping that is adequate for controling applications or demos. To dislay the result in parallel in a color map, we create a *nop* ('nop' in the context menu, which represents "no operation", i.e., a void operator, mostly just used for preview-purposes just like this) and leave the visualizer set to "map".

![com-part5](./img/tutorial/com-part5.gif)

To reduce sensor noise, we add a quick-fix using a *runningAvrg* ('math' > 'temporal' > 'runningAvrg'), which implements an exponential smoothing low-pass filter with x'=(x\*drag)+(x_p'\*(1-drag)), where x_p' is x' of the previous step. We set drag to 0.1, to not introduce too much latency.

Next, we want to make our processed data useable in a Unity3D scene. We prepared a Unity3D scene that receives OSC data via UDP using the UnityOSC addon. values are mapped to the Y-scaling of cubes in the Unity scene, for a quick visualization. In sqıd, we create an oscOut operator. We leave the protocol at the default of UDP, ID is set to loopback. We adapt the port to 6667 to match the socket we are using in our Unity code. Since we defined the OSC message string filter as "/unity", furthermore, we set device ID and sensor ID, which are at this point optional since they are ignored in our Unity code, and click "start" to start sending UDP datagrams. We can see in our Unity scene, that data is arriving and we are able to control Unity entities with our resistive sensors.

We save the scene, which writes all settings to disc, including COM ports and minima and maxima of our calibration phase. Next time scene is run, the device on COM8 is automatically connected and OSC sending is active right away. If nothing changed on the hardware end, there are no more user interventions required. Since we use UDP for OSC, there is no requirement for Unity to be up and running, we can start it anytime we want. We can also use different endpoints if we want, either in addition or replacing our Unity demo. 

# Appendix A: Examples

A few basic examples are listed here, intended to give an idea of intended workflow and sqıd's versatility and utility. More examples, use case scenarios, case studies, and additional content (somewhat overlapping with the content here) can be found in the [adjunct document](./pdf/sqid-adjunct.pdf) (PDF).

## TAFFI pinch detection and tracking
![taffi](./img/samples/blob-tracking.gif)
This sample uses web-camera live imagery to implement a very basic gesture pinch detection (on/off) based on the [TAFFI paper](https://doi.org/10.1145/1166253.1166292) by Wilson, including x/y coordinate control for dragging.
Using background subtraction with a static background model (captured image via `sample+hold`) in RGB-space and building the absolute difference, the hand pixels can be isolated as foreground area. Adding more ad-hoc image processing steps, including value scaling, multiplication of color channels, thresholding, and morphological opening, a distinct outline of the hand can be achieved. Connected component detection and tracking the center-of-mass can be used to generate a value representing on/off and a value pair representing x/y.
![taffi-detail](./img/samples//TAFFI-sbs.png)

## Hand Bend Direction Detection Using Myo EMG Armband
Using the (unfortunately discontinued) [Thalmic Myo](https://wearabletech.io/myo-bracelet/) gesture armband featuring 8 EMG sensors, a straightforward classification of hand bend direction is demonstrated: from the `myo` source operator, 8-channel EMG data is received that is plotted using a `nop` node for visual inspection. Absolute values are taken of the waveforms with an `abs` operator, a slight exponential smoothing filter is applied (α=0.05) to get rid of high-frequency noise with a `runningAvrg` operator. Individual sensor value ranges are then normalized using an `autoNormalize`, which was calibrated for minima and maxima with a few seconds of random input data. The 8 channel data were then split into two 2-channel segments, using two crop operators. Channels 3 and 4 (left) as well as channels 7 and 8 (right) were isolated, and those pairs were added together with a `sum` operator. Two `threshold` (t=0.5) operators were then used for left and right, respectively. Both values were joined into a single 2-value frame with a `join` operator and the result was sent to an UDP socket for utilization using an `oscOut`.
![myo-detail](./img/samples/myo-sbs.png)

## Multimodal input of Audio + MIDI + Myo EMG armband
![multimodal](./img/samples/myoaudiomidi.gif)

## Finger touch position tracking on (textile) touch matrix
![TexYZ](./img/samples/TexYZ.png)
Basic blob COM tracking for (multi-)touch tracking on a sensor matrix. The example was used for a demo of the CHI 2021 paper [TexYZ](https://doi.org/10.1145/3411764.3445479) by Aigner et al.
![fingertouch](./img/samples/finger-touch.gif)

## Kinect hand blob tracking
Basic hand COM tracking using depth image data (here, using the now discontinued Microsoft Kinect for XBox 360).
![kinect](./img/samples/kinect-tracking.gif)

## Receiving Google Soli live radar data via OSC
Although not a data processing demo, this example demonstrates a workaround to a common issue in handling prototype devices in UI research, which is dedicated operating system support. The API of the (now discontinued) Google ATAP [Project Soli](https://doi.org/10.1145/2897824.2925953) millimeter-wave radar sensor for gesture interaction provided compatibility with macOS and Ubuntu, but not Windows. Suppose the majority of your workflow depends on Windows (or vice versa, or some other OS), sqıd provides an easy means to transfer raw data from the Soli API (e.g., via a basic terminal application) to a different platform, where the remainder of the pipeline is located.
![google-soli](./img/samples/soli.png)

# Appendix B: Interoperability

Apart from the benefits of live patching and tuning, the main purpose of the modular architecture is the ability to replace source and sink device with little effort, which is facilitated by the network interfaces. sqıd is mostly relying on [OSC](https://opensoundcontrol.stanford.edu/) as a networking protocol, but other formats and/or interfaces are also possible (Serial, Bluetooth, MQTT, ZeroMQ, etc.). As a result there are numerous ways of interfacing external software, potentially running on dedicated. In the following, a few examples are provided.
![sample-setup](./img/example-setup.png)

## Interop w/ Unity3D via OSC protocol

Using the widespread [Unity3D](https://unity.com/) game engine, e.g., to implement a demo application or UI mockup, is an option that provides great versatility and flexibility, not only because it is very easy to learn and use, but also because it provides great portability, as it supports a great range of target platforms, such as Windows, macOS, iOS, Android, WebGL, and even game consoles. Hence, applications for desktop, mobile, gaming, and embedded platforms can easily be interfaced. See the repository [sqid-template-Unity3D](https://github.com/eyeco/sqid-template-Unity3D) for an example of how to easily get data into Unity3D.
![Unity3D](./img/3rdparty/Unity3D.gif)

## Interop w/ other data processing environments

In order to augment sqıd with additional data processing capabilities, one option beyond extending the codebase or programming plugins is to use existing 3rd party software that already provides the desired functionality. The [Python](https://www.python.org/) scripting language has gained popularity among the data scientists community in recent years, due to its accessibilty and extensive libraries. Evidently, also UI researchers use it frequently for tasks like data processing and pattern recognition, e.g., in the context of data filtering as well as interpretation, such as for gesture recognition. [MATLAB](https://www.mathworks.com/matlab) is another obvious candidate as it represents numerous toolboxes for diverse applications. Obviously, resulting data can be again returned to sqıd, if required, for further processing, or to forward the results to other distributed applications. This way, a bypass can be easily crated to benefit from a combination of multiple tools and their respective capabilities.
![example-bypass](./img/example-bypass.png)

### MATLAB via .m script

A simple method for getting data into MATLAB via OSC is provided in the repository [sqid-template-MATLAB](https://github.com/eyeco/sqid-template-MATLAB), which uses a MEX function that can be run in MATLAB script.

Furthermore, for larger data chunks that go beyond the limited datagram size, one can either use OSC via TCP, or use ZeroMQ instead.
![matlabAddon](./img/3rdparty/matlab.gif)

### Python script

See the [sqid-template-Python](https://github.com/eyeco/sqid-template-Python) repository for a straightforward boilerplate script to get sqıd data into Python script via OSC. 
![pythonAddon](./img/3rdparty/python.gif)

## Interop w/ Rhino 3D, Grasshopper via C# addon and OSC

[Rhinoceros 3D](https://www.rhino3d.com/), in combination with the visual programming addon [Grasshopper](https://www.grasshopper3d.com/), represents a powerful tool for 3D geometry that may be procedurally generated and parameter-controlled also by live data. See the [sqid-template-Grasshopper3D](https://github.com/eyeco/sqid-template-Grasshopper3D) repository for an exemplary Grasshopper3D plugin, that is receiving and parsing sqıd OSC data.
![ghAddon](./img/3rdparty/grasshopper.gif)

## Other options

Beyond the provided examples, there are numerous other options to combine sqıd with software that may be useful or essential for certain scenarios. Another repository [provides boilerplate code](https://github.com/eyeco/sqid-template-Processing) for the [Processing](https://processing.org/) graphics library. Further supposable options are [vvvv](https://vvvv.org/) for procedural graphics or to operate complex multimedia installations, [Pure Data](https://puredata.info/) for interactive audio control, [ROS](https://www.ros.org/) for robotic systems, and many more. Basically, anything that can be equipped with a network socket can be a turned into a receiver or sender. 

![P3](./img/3rdparty/P3.gif)

## Networking Interface and Format
Note that all the shown examples and implementation use the sqıd custom OSC format for a reasonably compact data transfer of moderately sized data arrays or matrices (*SampleFrames*), which&mdash;under the hood&mdash;send blob data, in combination with metadata like matrix dimensions and timestamps. This is the main reason for the *plugins* and *addons* that are shown here, basically representing custom OSC parsers, which basically require the 3rd party technology to be extendible in some way. However, depending on the nature of the data, it may also be sent via standard OSC, which opens the door to interface also 3rd party software that cannot be altered or extended that easily, such as [MadMapper](https://madmapper.com/) or [Ableton Live](https://www.ableton.com/en/live/). This extends the potential further, as OSC is a protocol supported by countless applications. Incidentally, all the previously shown examples can also be operated this way. Ultimately, what is the better option is a matter of the use case scenario at hand.
![vvvv](./img/3rdparty/vvvv.gif)

To provide OSC parsers for sqıd's own SampleFrame protocol, the format is described here: an OSC message is of arbitrary length and starts with four `int32`, which are, however, filled by unsigned integers and must be interpreted accordingly. Unfortunately, [standard OSC](https://opensoundcontrol.stanford.edu/spec-1_0.html) is somewhat limited in terms of data types as it has no concept of unsigned and 8 or 16 bit types. The four integers represent the data matrix's width (*w*), height (*h*), and depth (*d*), in this order, as well as a timestamp *ts* (milliseconds). This header is followed by a `blob`, which in OSC is a term describing a byte block of arbitrary data. The SampleFrame blob contains *n = w* × *h* × *d*  IEEE 754 32 bit single precision floats, i.e., *n* × 4 bytes. Values are in row-major order. In case of *d* > 1, values are interleaved.

The format used for sending SampleFrames via ZeroMQ is similar, except width, height, and depth are represented by `uint16_t` data types and timestamp is of `uint32_t`.

# Appendix C: Features

## Core

### Supported protocols, interfaces, and networking
- generic serial port data via [serial](https://github.com/wjwwood/serial)
- generic Bluetooth Serial data via [bluetooth-serial](https://github.com/Agamnentzar/bluetooth-serial-port)
- OSC (UDP+TCP) via [liblo](http://liblo.sourceforge.net/)

### Frame compression (lossy + lossless)
- LZO via [MiniLZO](http://www.oberhumer.com/opensource/lzo/#minilzo)
- QLZ via [QuickLZ](http://www.quicklz.com/)
- BZ2 via [bzip2](https://www.sourceware.org/bzip2/)
- zStD via [zStd](https://facebook.github.io/zstd/)
- zLib via [zLib](https://zlib.net/)
- LZ4 via [LZ4](https://lz4.github.io/lz4/)
- JPEG via [libjpeg-turbo](https://libjpeg-turbo.org/)

## Plugins

### Supported devices
- XInput based gamepads via [NvGamepad](https://developer.nvidia.com/cross-platform-gamepad-api)
- Leap Motion The Leap raw camera and hand skeleton input via [LeapSDK](https://www.ultraleap.com/)
- Sensel via [senselLib](https://github.com/sensel/sensel-api)
- multichannel audio input via [RtAudio](https://www.music.mcgill.ca/~gary/rtaudio/)
- Thalmic Labs Myo EMG armband via [Myo SDK](https://support.getmyo.com/hc/en-us/articles/360018409792-Myo-Connect-SDK-and-firmware-downloads)
- Microsoft Kinect depth, RGB, and skeletal data via [Kinect for Windows SDK 1](https://www.microsoft.com/en-us/download/details.aspx?id=40278)
- OptiTrack raw camera input via [OptiTrack Camera SDK](https://optitrack.com/software/camera-sdk/)
- Intel RealSense depth an RGB camera input via [librealsense2](https://github.com/IntelRealSense/librealsense)

### Supported protocols, interfaces, and networking
- IPC via via [ZeroMQ](https://zeromq.org/)
- MQTT via [Eclipse Mosquitto](https://mosquitto.org/)
- CAN via [Peak System PCAN-Basic API](https://www.peak-system.com/Development.526.0.html)
- TUIO 2.0 via [TUIO20_CPP](https://github.com/mkalten/TUIO20_CPP)
- MIDI via [RtMidi](https://github.com/thestk/rtmidi)
- OptiTrack NatNet marker and rigid body input via [OptiTrack NatNet SDK](https://optitrack.com/software/natnet-sdk/)


# Appendix D: Operators

_TODO: add detailed description of each of the ops and their parameters_

## List of sources (core)

- sources
	- COM
	- RFCOMM
	- OSC

## List of operators (core)

- color
	- convert
	- HSVshift
	- toGrayscale
- devices
	- capture
	- sensor
	- serialOut
- fileIO
	- fileIn
	- fileOut
	- imageIn
	- imageOut
	- videoIn
	- videoOut
- gnr8
	- const
	- noise
	- signal
- imaging
	- blur
	- contourDetector
	- highPass
	- lowPass
	- morph
	- opticalFlow
	- tracking
		- blobTracker
- math
	- abs
	- add
	- addConst
	- approx
	- boolean
		- and
		- or
		- xor
	- clamp
	- clampConst
	- exp
	- expConst
	- inRange
	- invert
	- linear
	- linEq
	- linEqConst
	- log
	- matrix
		- determinant
	- max
	- maxConst
	- min
	- minConst
	- mix
	- mixConst
	- mul
	- mulConst
	- multiLinear
	- powConst
	- product
	- sigmoid
	- slope
	- spectral
		- fft (1D)
		- fft (2D)
	- sqrt
	- statistics
		- centerOfMass
		- histogram
	- sub
	- sum
	- temporal
		- bgSubtraction
		- box
		- drag
		- integral
		- kalman
		- mean
		- median
		- resample
		- runningAvrg
	- threshold
	- transform
		- autoNormalize
		- normalize
		- remap
		- toPolar
- networking
	- oscOut
- nop
- pointClouds
	- projectTo3D
- util
	- buffer
	- crop
	- flatten
	- flip
	- flipFlop
	- join
	- maxPooling
	- merge
	- onOff
	- reshape
	- resize
	- s+h
	- sampler
	- split
	- sync
	- time
	- timestamp
	- transpose

## List of operators (plugins)

- audio
	- power
- devices
	- audioIn
	- gamepad
	- kinect
	- leap
	- midiIn
	- midiOut
	- myo
	- OptiTrack
	- RealSense
	- sensel
- networking
	- canIn
	- canOut
	- mqttIn
	- mqttOut
	- zmqIn
	- zmqOut
	- natnet
		- natnet
		- pf2sf
		- mf2sf
	- tuio
		- tuio
		- toSampleFrame
