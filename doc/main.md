# User Documentation

## Supported features

Some features are selectively built into the main module, according to pre-processor switches. Refer to the console output to learn about included features e.g.
```
version 0.1.2
Release build for AMD64 with C++ language standard v201704
built with MSVC v1916(191627045) at Wed Jul  3 14:13:39 2024 with:
  GUI support                  YES
  Bluetooth Classic support    YES
  FFTW support                 YES
  Compression support          NO
```

Addons (such as support for MQTT, Kinect, Myo, etc.) are built as separate dll modules and dynamically loaded by the core module at startup (if configured to be loaded in the [config.json](../config.json) file). More details about how to do this will follow.

For replicating the serial communication from an ESP via USB, with the [firmware code](../firmware/) included in this package, find the USB to UART Bridge Virtual COM Port (VCP) drivers [here](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads).

## General

The software is not demanding in terms of hardware specification – for reference, we easily run it on an Intel NUC (NUC6i7KYB) with Intel Core i7-6770 @ 2.6GHz, 16GB RAM, and integrated graphics Intel Iris Pro Graphics 580, although weaker CPUs and less RAM may work as well.

## User guide

### How to run

Usage: ```sqid [-g | --gui] [-h | --help] [sceneName]```

**Note: Make sure the working directory is the one containing the folders ```resources``` and ```shaders```, since those will be sought at ```./```, i.e., run the program from this directory.**

| Option            | Description                                       |
|-------------------|---------------------------------------------------|
| ```-g, --gui```   | run with GUI (optional)                           |
| ```-h, --help```  | print help (optional)                             |
| sceneName         | name of scene, without file extension (optional, defaults to 'scene')  |

Example: ```sqid -g myScene``` will run sqid with GUI, loading the scene configuration from ```myScene.json```, the UI layout from ```myScene.ui.json```, and the window position/size, as well as preferences (auto-save, view settings) from ```myScene.ini```. If the scene file does not yet exisit, it will be created. Scenes can be placed in subdirectory; in that case specify the absolute or relative path, e.g., ```myDir/myScene```. **Note that directories must exist, they will not be created!** Since all required information is contained in the respective .json, .ui.json, and .ini files, scenes can be easily copied or moved just by copying or moving these three.

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
| ```Ctrl + q```    | quit (without saving, unless autosave is on) |
| ```Ctrl + s```    | save scene (scene file name and path as specified at startup) |
| ```Ctrl + c```    | copy selected sub-graph to clipboard |
| ```Ctrl + v```    | paste from clipboard          |
| ```Space```       | show/hide scene graph (to clear view when visualizer is maximized, see below) |
| ```F1```          | open finder                   |
| ```Esc```         | close finder                  |
| ```Delete```      | delete selection              |

### User Interface

![name-collapse](./img/name-and-collapse.gif)

Operators consist of a name, a visualizer (live-preview of the data), input pins and/or output pins. Red color on an output pin signals activity, i.e., frames being pushed downstream. Names specify to the operator type by default, but can be overridden to custom string using the ```name``` field in the inspector. Nodes can be collapsed to save space (and performance for visualizer drawing) and restored using the ```-```/```+``` toggle button on the left edge. 

![tooltip](./img/tooltip.png)

Hovering the mouse over an output pin displays a tool-tip overlay, giving the pin name, followed by the number of queued frames, the data type, and type-specific information. E.g., the output pin "out" of the video capture operator in the example below shows holds 1 frame of type ```sf``` (generic SampleFrame), of dimensions 640x480x3. Timestamp is 6294275, containing values of range [0 1] (min/max values in current frame).

![tooltip](./img/deactivate.gif)

Operators can be deactivated by unchecking the ```enabled``` checkbox in the Inspector, which is visualized by darkened nodes. This entirely stops the processing and therefore the output at the pins (blue: no frames are getting pushed). Operators can also be deactivated by code, in case there is an unresolvable error in the processing step, e.g., when faulty or incompatible frames are fed as input. In this case, the error must be resolved before the operators can be enabled again. Note the following example, where frames of different sizes (4x4 vs. 5x4) are fed to multiplication and subtraction operators, causing errors in both, sending them to disabled state.

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

Create operators using the finder: hit ```F1``` and start typing the name. Select the operator type in the list that appears and hit ```Return```.

![delete](./img/delete.gif)

Delete one or multiple selected operators by hitting the ```Delete``` key.

![copy-paste](./img/copy-paste.gif)

Copy and paste one or multiple selected operators using the menu ('Edit' > 'Copy', 'Edit' > 'Paste') or the keyboard shortcuts ```Ctrl+c``` and ```Ctrl+v```. The copy process clones all operators including their configuration and the subgraph's internal patching.

### Patching

![patch](./img/patch.gif)

Connect connect operators' outputs to inputs by dragging output pins to compatible (green) input pins (left-click). Output pins can be connected to multiple inputs, but not the other way around.

![repatch](./img/repatch.gif)

Connections can be cleared by left-clicking connected input pins. If an output is connected to an already connected input pin, the established connection will be replaced.

## Tutorial: how to set up to retrieve data from an MCU via serial port

This is a walkthrough for the task of setting up a very simple input that comes from an ESP that is connected via serial port, just to get you started with the very basics. The ESP is running the firmware that is also included in this package in the folder [ESP_firmware](../firmware/).

_Note that during a recent code refactoring step, there was a change in terminology made: previous **sources** are now called **interfaces**, since they will (and partly already do) support writing/sending data, instead for just reading from an interface, such as COM and BTS. Sending is supported using the new `Sink` operator. Consequently, what previously was termed a `sensor` operator is now called `Source` operator, which is more generic and therefore more adequate anyway._

### Step 1: set up COM input
First, we create an "interface" (previously called "source") node, which is not an operator, but an entity that is required to retrieve arbitrary data from devices that can send arbitrary data. Since we cannot know how many sensors an MCU is sampling, and moreover, how many sub-devices it may be operating, we introduced the concept of "Device IDs" and "Sensor IDs" to distinguish. In the firmware code you can see that six ADCs are sampled (```multiSample()```) and the results are stored into a float array, which is copied into the frame stucture (via ```frame.setData()```) and then sent via serial port (```sender.send()```).

```
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
```

Note that the frame was earlier configured as float array and associated with the sender instance during setup (```sender.init()```):

```
SampleFrame frame( DEVICE_ID, SENSOR_ID, SampleFrame::L_ARRAY, SampleFrame::DT_FLOAT, ANALOG_IN_COUNT );
SenderSerial sender;

//...
void setup() {

  //...
  if( !sender.init( &frame ) )
    Serial.println( "failed to init pointSender" );
  //...
}
```

This means that even tough we sampled six ADCs in total, we treat the data as a single 6-value array to send it via serial in a more compact way. For identification within sqid, we defined Device ID as 0 and Sensor ID also as 0.

Note that all of these are implementation details. All the user really has to know is that there is data coming in which is associated with a certain combination of Device ID and Sensor ID, the rest is done within sqid.

![com-part1](./img/tutorial/com-part1.gif)

We create an interface (previously called _source_) node for serial using `interfaces > COM` in the context menu (previously `source > COM`). Alternatively open the finder with F1 and type "com" to bring it up. We select it, and configure using the inspector: we select the COM port from the dropdown list (since we just plugged the MCU, it does not yet show up, so we refresh the list by hitting "rescan"). We may modify baud rate and number of maximum queued packages, then press "open". We can see in the inspector that data is coming in ("started" and "synced") and that there are packages coming from one senders. By expanding the group, we can see Device IDs (dID) and Sensor IDs (sID) of queued frames. 

Next, we create an operator of the generic type *source* (previously called *sensor*), which is used to selectively grab frames from the interfaces. In the inspector, we associate it with input type `COM`, set it to receive from port 8, and specify Device ID and Sensor ID of the frame we're interested in (i.e., 0 for both). We can see in the node visualizer that data is coming in, as it changes from black to a heatmap visualization. 

![com-part2](./img/tutorial/com-part2.gif)

Since in this example we attached four sensors, but the firmware is sampling six ADCs, the last two floats of the array are not in use and stay at 0. Hence, we are only interested int the sub-array of elements [0...3] and want to operate with a 4x1 frame from here on. We create a *crop* operator (`util > crop`) and set 'right' to 0.75, to crop to the leftmost 75% (technically 2/3 would be correct, but the width is floored to the 4 anyways). When we hover the mouse cursor over the output pin, we can see that the output is of size '4x1'.

We want a signal that is 0 when there is no activity, and reaches up to 1 for full saturation. Since we used resistive sensors in a voltage divider, the voltage drops when they are actuated, so signal is actually upside-down: high voltage at inactivity, low voltage when they're actuated. The most simple way to fix this is to create an *invert* operator (`math > invert`), which transform each frame element by x'=(1-x). For better clarity, we switch the visualizers to temporal line drawing ('line (t)') to get a temporal tend of the signal.

![com-part3](./img/tutorial/com-part3.gif)

By inspecting the min/max markers in the visualizer, we see the inverted signal values are ~0.21 at rest and ~0.87 at saturation; also, the array sensors' min and max values differ slightly. For a quick'n'dirty calibration want to map them to ranges of about [0 1]. The *autoNormalize* operator (`math > transform > autoNormalize`) is meant to facilitate this. To better utilize the line visualizer's displayed range, we offset drawing by -1 and scale by 2, thus setting the drawing range from the default [-1 1] to [0 1].

![com-part4](./img/tutorial/com-part4.gif)

it can use a "learning phase" to discover minima and maxima of either (i) individal frame elements or (ii) of the entire frame. The elements are transformed by x'=(x-min)/(max-min). We want each element to be mapped individually, so we leave the radio button at "individually". We check the "learn" checkbox to activate learning phase and actuate each of the sensors to maximum, one after the other. After this calibration phase, minima and maxima were recorded for each frame element. To no longer have them modified during operation, we uncheck the "learn" checkbox, so they are set. 

We see in the autoNormalize node, that the range of [0 1] is reasonably exhausted, giving us now a value mapping that is adequate for controling applications or demos. To dislay the result in parallel in a color map, we create a *nop* ('nop' in the context menu, which represents "no operation", i.e., a void operator, mostly just used for preview-purposes just like this) and leave the visualizer set to "map".

![com-part5](./img/tutorial/com-part5.gif)

To reduce sensor noise, we add a quick-fix using a *runningAvrg* (`math > temporal > runningAvrg`), which implements an exponential smoothing low-pass filter with x'=(x\*drag)+(x_p'\*(1-drag)), where x_p' is x' of the previous step. We set drag to 0.1, to not introduce too much latency.

Next, we want to make our processed data useable in a Unity3D scene. We prepared a Unity3D scene that receives OSC data via UDP using the UnityOSC addon. values are mapped to the Y-scaling of cubes in the Unity scene, for a quick visualization. In sqid, we create an oscOut operator. We leave the protocol at the default of UDP, ID is set to loopback. We adapt the port to 6667 to match the socket we are using in our Unity code. Since we defined the OSC message string filter as "/unity", furthermore, we set Device ID and Sensor ID, which are at this point optional since they are ignored in our Unity code, and click "start" to start sending UDP datagrams. We can see in our Unity scene, that data is arriving and we are able to control Unity entities with our resistive sensors.

We save the scene, which writes all settings to disc, including COM ports and minima and maxima of our calibration phase. Next time scene is run, the device on COM8 is automatically connected and OSC sending is active right away. If nothing changed on the hardware end, there are no more user interventions required. Since we use UDP for OSC, there is no requirement for Unity to be up and running, we can start it anytime we want. We can also use different endpoints if we want, either in addition or replacing our Unity demo. 

# Appendix A

## Example: TAFFI pinch detection and tracking
![taffi](./img/samples/blob-tracking.gif)

## Example: Kinect hand blob tracking
![kinect](./img/samples/kinect-tracking.gif)

## Example: Multimodal input of Audio + MIDI + Myo EMG armband
![multimodal](./img/samples/myoaudiomidi.gif)

## Example: Finger touch position tracking on (textile) touch matrix
![fingertouch](./img/samples/finger-touch.gif)

## Example: Receiving Google Soli live radar data via OSC
![google-soli](./img/samples/soli.png)

# Appendix B

## Interfacing w/ vvvv beta via custom addon and OSC
![vvvv](./img/3rdparty/vvvv.gif)

## Interfacing w/ Rhino 3D, Grasshopper 3D via C# addon and OSC
![ghAddon](./img/3rdparty/grasshopper.gif)

## Interfacing w/ MATLAB via .m script and ZeroMQ
![matlabAddon](./img/3rdparty/matlab.gif)

## Interfacing w/ Processing via oscP5 library 
![vvvv](./img/3rdparty/P3.gif)

# Appendix C

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


# Appendix D

_TODO: add detailed description of each of the ops and their parameters_

## List of interfaces (core)

- interfaces
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
	- source
	- sink
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
