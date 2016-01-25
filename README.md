#LEDWax for Particle Photon

##Cloudy LEDs

An IoT LED controller for Particle Photon with support for PWM LEDs and WS28xx LED Strips.

## Features
* Efficiently control arrays of LED strips from a single Photon.
* Set LED colors.  Millions of choices.
* Set LED brightness.  1024 (10-bit) choices.
* Many display modes for single and multi-pixel LED strips.
* Animated display modes.
* Remembers previous settings when powered-up.
* Native color fading.
* IoT Enabled - REST API for control.
* Supports Particle Photon.
* Designed and built to drive piles of LEDs.
* Support for single-color PWM LED strips.
* Support for multi-color (RGB+, PWM) LED strips.
* Support for many types of (SPI) Addressable LED strips, such as WS2801, WS2811, and WS2812.
* Support for I2C PWM LED controller (PCA9685).
* Support for PWM output from Photon's PWM pins (native PWM).
* Non-blocking, time-sliced firmware.

## Setup

1) Connect LEDs or LED strips to a Particle Photon.
2) Configure the software and flash it to the photon.
3) Control the LED strip using the LEDWax REST API

NOTE: Sparkfun WS2801 strip support is restored but untested.

### Firmware Configuration
LEDWax uses a configuration object to setup the firmware.  This object, fields, and constructor can be viewed in ledwax_photon_config.h.

Edit the code in application.cpp.  For example, where we have two strips, one PWM and another WS2812-type (2 strips):
```cpp
// *********** EDIT THIS SECTION ACCORDING TO HARDWARE ***********
config[0].setStripType(STRIP_TYPE_WS2811);
config[0].setNumPixels(120);
config[0].setNumColorsPerPixel(3);
config[0].setSpiPins(new uint8_t[WIRE_NUM_SPI_1_WIRE]);
config[0].getSpiPins()[0] = A5;
config[0].setMatrix(true);
config[0].setMatrixHeight(8);

config[1].setStripType(STRIP_TYPE_I2C_PWM);
config[1].setNumPixels(1);
config[1].setNumColorsPerPixel(NUM_PIXELS_PER_LED_PWM_RGB_STRIP);
config[1].setI2cPwmPins(new uint8_t[NUM_PIXELS_PER_LED_PWM_RGB_STRIP]);
config[1].setI2cAddy(0x70);
config[1].getI2cPwmPins()[0] = 0;
config[1].getI2cPwmPins()[0] = 1;
config[1].getI2cPwmPins()[0] = 2;
config[1].setMatrix(false);

config[2].setStripType(STRIP_TYPE_NATIVE_PWM);
config[2].setNumPixels(1);
config[2].setNumColorsPerPixel(NUM_PIXELS_PER_LED_PWM_RGB_STRIP);
config[2].setNativePwmPins(new uint8_t[NUM_PIXELS_PER_LED_PWM_RGB_STRIP]);
config[2].getNativePwmPins()[0] = RX;
config[2].getNativePwmPins()[0] = TX;
config[2].getNativePwmPins()[0] = WKP;
config[2].setMatrix(false);
// *********** END EDIT THIS SECTION ***********
```
Note:  Please refer to ledwax_photon_config.h, ledwax_photon_constants.h, and spark particle application.h for useful constants.

#### SPI-based LED Strip (NeoPixel, WS2801, WS2812, etc.)
In the above example, the first strip is an addressable WS2811 strip (WS2801-type) that is connected to an SPI port.  It has 120 pixels with 3 colors per pixel (RGB).  The strip is split into 8 sections for animated display modes.  This allows for display of 2-D sprites on the strip to create interesting effects, such as mode 32.

In general, connecting an addressable LED-strip always requires a separate, rated, power supply.  Refer to the distributor or manufacturer datasheet.  __SPI-based LED strips connected to the Photon may require a voltage-level shifter for the SPI clock/data/select lines__.  The Photon has 3.3v GPIO and many addressable WS2801-type strips require 5v.

#### I2C-based LED Strip (PCA9685)
In the configuration example, the second strip is connected to a PCA9685 I2C PWM chip.  It has 1 pixel.  The PWM strip in this example is an RGB strip - so it has one (1) pixel with 3 colors.  It is *not* setup as a matrix.  There are settings for the I2C bus address.  The default I2C pins will be used to drive the PCA9685 chip.  There are settings for the output PWM pins on the PCA9685 chip.

It is important to note that on a (typical) PWM LED strip, the number of LEDs *is not* the same as the number of LEDs.  A pixel is individually controllable, and each pixel on the strip is not individually controllable.  On a PWM strip, all of the LEDs take the same color, so there is only 1 pixel.

LEDWax supports multiple I2C-based PWM drivers with support for I2C addressing.  I2C devices can be chained together and given distinct addresses.  Additional LED strips can be configured in firmware for the attached I2C devices.

In general, using a PCA9685 IC to drive LED-strips with a Photon always requires a separate power supply for the LEDs and/or LED-strip.  The PCA9685 is powered with 5v, which can be the same 5v used to power the Photon.  The PCA9685 data bus supports 3.3v GPIO (from the Photon), so a voltage-level shifter should not be required (to be verified).

Driving a small number of LEDs connected directly to a PCA9685 is possible, since it can sink more current than a Photon.  However a power supply that can handle the load is still required.  Check the current requirements on the LED datasheet.

Driving LED-strips with a PCA9685 typically requires a MOSFET circuit.  The PCA9685 is used to drive the FET gate pins high/low to permit current to flow.  N-channel FETs work well with common-anode LED strips.  With this hardware, the common anode is wired to a seperate 12v/24v power supply.  Other voltages are fine as long as the FET is rated for it.  Each LED cathode is then wired through the FET, so the circuit is closed (to ground) when the FET gate is driven high.

#### Native PWM LEDs
The third strip (actually, just a single LED) is connected directly by the Photon's PWM pins.  This requires a MOSFET (or equivalent) power driver circuit.  Be careful of power requirements!  You can drive a single LED - of 1 colors, maybe more, without additional power.  The LED must not pull too much current from the Photon - see the datasheet for the Photon and your LED to determine if it's within a safe range.  The LED that comes with the Sparkfun Photon development kit is OK.

In general it is difficult to use native PWM alongside SPI and/or I2C LED controllers.  This is due to the limitations on power draw.  There are also a limited number of PWM output pins.  Finally, several of the PWM output pins conflict with SPI and I2C pins (interfaces) on the Photon.  *It is not possible to double-purpose a pin.*  For example it is not possible to configure a pin to be used for SPI and native PWM.

Native PWM output from the Photon can be combined with a MOSFET circuit, as with a PCA9685, to drive the maximum number of LEDs with a seperate power supply.  The same circuit (i.e. fet-board) can be used in both configurations.

# Firmware Usage
Setup LEDWax Photon with a LED strip and a Particle Photon.  Then control the strip using the exposed REST API.  Send HTTP POST commands to change color, display mode, animation, etc.  Issue HTTP GET requests to discover LEDWax configuration data, such as number of strips, display mode, colors, etc.

There are only 2 or 3 basic concepts you need to understand for controlling the strips.  First, there is a "mod" command that tells LEDWax how many colors to display and how to cycle through them.  Sending a valid "mod" command will cause the value of the "ledDisplayMode" particle variable to be updated.

Several display modes are supported.  Their behavior varies but there are three (3) basic families of display modes.  In "family 1" the user can configure up to 3 different colors which can be displayed in different ways on the strip.  In "family 2" the colors are pre-set, and the user may have the option to customize a parameter of the display mode (for example:  cycle through a rainbow of pre-defined colors, user-defined length of time to cycle).  A "family 3" display mode is an animated (sprite-based) display mode, for example the "cylon" and "dot" display modes.  These display modes are only compatible (currently) with SPI-based addressable LED-strips.

If the displayMode parameter is set to a "family 1" mode, then colors can be customized using the "col" command.

If the displayMode parameter is set to a "family 2" mode, then the multiColorHoldTime and ledFadeTime may or may not have an impact on the display behavior, depending on the mode.

If the displayMode parameter is set to a "family 3" mode, then the ledFadeTime typically impacts the display behavior, depending on the mode.  For example, in cylon mode is adjusts the speed of the moving pattern, with lower values being faster (less delay).

Understanding the above concepts will allow for basic control using raw commands.  There are more commands and variables for finer control and customization.

# LEDWax IoT REST API
LEDWax-Photon is built for the Photon so REST API [variables](https://docs.particle.io/reference/api/#get-a-variable-value "particle variables") and [functions](https://docs.particle.io/reference/api/#call-a-function "particle functions") are accessed through the Particle Cloud.

## [Variables][particle-vars]
The following particle cloud variables are exposed:
* "numStrips": "int32" - number of LED strips configured in firmware
* "stripIndex": "int32" - current strip being controlled 
* "stripType": "int32" - type of (current) strip
* "dispMode": "int32" - display mode of (current) strip
* "modeColor": "string" - A a JSON array representing the colors assigned to current mode of (current) strip.  For example, [FF, A5D5, 00].
* "modeColorIdx": "int32" - index of modeColor array currently displayed on the (current) strip
* "brightness": "int32" - brightness of (current) strip
* "fadeMode": "int32" - color transition mode of (current) strip
* "fadeTime": "int32" - time spent on color transitions of (current) strip
* "colorTime": "int32" - time spent displaying each color assigned to color mode

## [Functions][particle-funcs]
The following particle cloud functions are exposed:
* setLEDParams(String command) : send command to LED strip
* resetAll(String command) : reset all LED strips

#### resetAll(String command)
Resets all LED strips to firmware defaults.  Stores firmware defaults in EEPROM also, so they remain until re-configured.

#### setLEDParams(String command)
The format for "command" is:
> [command-name];[cmd-value]?[,cmd-value]*

There is no space between the command-name and cmd-value(s).  All commands require a cmd-value - except "qry".  Some commands accept more than one cmd-value, such as "col" (color).  Multiple cmd-values are comma-separated.  Commands are terminated with C++ string termination (\0).

"command-name" can be one of the following:
>	qry : TBD

>	idx : set stripIndex - the current strip being controlled.  All following commands will be executed against this LED strip.  Valid values are 0 - (NUM_STRIPS - 1).  Default is 0.

>	col : set modeColor - the LED pixel color.  cmd-value must ahere to the following format:

>		[mode-color-index],[24-bit-integer]
>	where mode-color-index is the index of the mode color (family 1 display mode) to set
>	valid color values are 0 - 16777215 (24-bit integer)

>	brt : set brightness - the strip brightness. Valid values are from 0 (0% = full off) to 1023 (100% = full on).  NOTE:  even though brightness is stored as a 10-bit value, it is currently displayed as an 8-bit value.  Thus a value of 1023 in 10-bit storage corresponds to a display value of 255 in brightness.  Brightness is stored sepearately from color in firmware.

>	mod : set dispMode - the strip display mode.  Valid values are:

>		0. solid color (default) : 1 user-defined color displayed on strip
>		1. fade terawatt industries colors : display TW colors, across entire strip - alernate entire strip bewtween colors
>		2. fade random colors : display 2 random colors, across entire strip - alernate entire strip bewtween colors
>		10. fade two colors : display 2 user-defined colors, across entire strip - alernate entire strip bewtween colors
>		11. fade three colors : display 3 user-defined colors, across entire strip - alernate entire strip bewtween colors
>		12. two alternating colors : 2 user-defined colors, displayed on alternating pixels
>		13. terawatt industries alternating colors : TW colors, displayed on alternating pixels
>		14. three alternating colors : 3 user-defined colors, displayed on alternating pixels
>		15. two random alternating colors : 2 random colors, displayed on alternating pixels
>		16. three random alternating colors : 3 random colors, displayed on alternating pixels
>		20. rainbow : cycle whole strip through a rainbow of color
>		21. rainbow cycle : same as above, but distribute the rainbow across strip
>		22. random candy : random colors moving across the strip, one LED at a time
>		30. cylon (animated) : animated cylon; works best with matrix height = 1 = whole strip
>		31. dot (animated) : animated dot; works best with matrix height = 1 = whole strip
>		32. square (animated) : 2-D square on a 1-D strip, works best with matrix height >= 6

>		Display modes marked with (animated) currently only work with addressable (WS2811-type) strips.  Adjusting matrix settings will change how each animated mode is displayed.  The speed of motion can be controlled with the "lfti" command of the "setLEDParams" endpoint (below).  The color of the animation can be controlled with the "col" command.  Most animations will only use the first color mode setting.  Brightness can be adjusted normally.

>	mht : Set holdTime - the multi-color-hold-time.  The multi-color-hold-time determines how long each color is displayed before the transition to the next color.  Valid values are 0 - 65535 (16-bit integer).

>	lfm : Set fadeMode - the led-fade-mode.  The led-fade-mode is the style of transition between colors, and only applies to certain display modes.  A value of 0 enables the native-fade color transition, which causes the color of the entire strip to smoothly transition from one color to the next.  A value of 1 disables the native-fade transition, which causes each pixel along the strip to switch from one color to next.  This setting currently only applies to display modes 0 - 10.  Valid values are 0, 1.  The time spent during transition is defined by the ledFadeModeTimeInterval variable, and can be adjusted with the 'lfti' command.

>	lfti : Set fadeTime - the led-fade-mode time-interval.  The LED-fade-mode time-interval defines the duration of the LED color transition, in milliseconds.  Valid values are 0 - 65535 (16-bit integer) 

#### Example Commands
* idx;0
* mht;5000
* mod;1
* mod;2
* col;0,255
* col;1,127
* col;2,64
* lfm;1
* mod;22
* brt;0
* brt;512

## RestAPI Examples
Request Type | Request | Response
:-------------|:---------|---------:
POST | curl http://[cloud-ip-address-with-port]/v1/devices/3800XXXXXX473XXXX231XXXX/setLEDParams -d access_token=[access-token-from-particle-token-list] -d args="idx;1" | {  "id": "3800XXXXXX473XXXX231XXXX",  "name": null, "last_app": null,   "connected": true,   "return_value": 0 }
POST | curl http://[cloud-ip-address-with-port]/v1/devices/3800XXXXXX473XXXX231XXXX/setLEDParams -d access_token=[access-token-from-particle-token-list] -d args="col;0,255" | {  "id": "3800XXXXXX473XXXX231XXXX",  "name": null,  "last_app": null,   "connected": true,   "return_value": 0 }
GET | curl http://[cloud-ip-address-with-port]/v1/devices/3800XXXXXX473XXXX231XXXX/colorTime?access_token=[access-token-from-particle-token-list] | {  "cmd": "VarReturn",  "name": "colorTime",  "result": 19,  "coreInfo": {    "connected": true   } }

# Hardware
Setting up a circuit with some LEDs isn't too difficult.  If you've never setup a LED circuit from scratch then checkout the fun products at Adafruit and Sparkfun for help.  If time permits an example circuit diagram and sketch configuration will be provided.

The latest version of this firmware is intended for a PWM driver IC to extend the PWM output capabilities of the Photon.  The PWM driver should connect to the I2C interface of the photon.

This project is intended for serious amateur usage.  The user of this software assumes all resposibility.  By using this software you agree to its terms and conditions.  See LICENSE.txt.

I've designed a custom PCB which is undergoing testing so this can be assembled more easily.  The PCB makes it easier to setup different types of LED strips with a Photon.

# Efficiency
LEDWax is designed and built to support a heap of LEDs and LED-strips of varying types.  The primary focus is on support for LED-driver chips, and particularly those chips which independently maintain state - such as WS2801 and PCA9685.  This means LEDWax can issue a command to set color/brightness to a LED driver, and the driver holds that setting until it gets a new command.

LEDWax takes advantage of this architecture to reduce the amount of work it has to do.  This means it can drive more LED-strips with smoother animations.  It should be capable of driving hudreds, perhaps thousands, of individual pixels (TBD).

Still, there are limitations.  I'm not sure how many strips it can drive simultaneously before noticeable latency occurs.  Actually, the number of strips isn't the main constraint, but rather - the number of distinctly addressable pixels defined in the strip firmware configuration.  Each pixel gets its own memory space and color processing.  The actual limits on memory are unknown (TBD), as are baseline processing costs, but these are the heaviest factors.

Once pixel memory has been processed, which only happens when it needs to, then it's output to the connected interface - SPI, I2C, or native PWM.  The amount of time spent sending signals on the output interfaces may be non-negligible.  If SPI-bus interface-speed is 8mhz, then in theory the bus can transmit 125kB/s, and if each LED on a strip takes a byte of data, then in theory an SPI interface can address 125k LEDs per second.  The same goes for I2C.  However the CPU is busy processing memory and data.  Maximum efficiency is achieved when SPI and/or I2C interfaces use the builtin hardware-based GPIO provided by the Photon.

NOTE:  Unfortunately the FastLED (and I2C?) libraries use software-based GPIO.  This means LEDWax must wait for the library to clock all of the data to the target bus (SPI).  The time spent clocking the data is proportional to the number of pixels attached to the bus.  In the example above with 8mhz bus, using a 60-LED RGB WS2811 strip, the time spent clocking data is at least sixty microseconds (60us).
***
# Develop & Contribute
LEDWax-Photon is a C/C++ project targeted at ARM GNU EABI cross-tools compiling, specifically for the Photon/STM platform.  To compile this software from scratch you need a cross-compiler toolchain, such as the ones found at https://launchpad.net/gcc-arm-embedded/.

The "master" branch shall be considered stable, but it's currently just a stub.  As of version 0.2, no branch is considered stable.  Version 0.2 is the latest as of 01/2016.

## Architectural Concepts
The LEDWax-Photon firmware is built with a custom temporal multi-threading model.  There should never be a need to use delay().  Instead of delays, new features should rely on state-machines that have the same effect.  There are currently two state machines.  They can be re-used for new features, or new state machines can be added.

The second major concept utilized by LEDWax is to prefer runtime over compile-time configuration.  This has a major impact on design decisions and runtime performance risks.  For example it impacts decisions such as class architecture and memory management.  Hardcoded variable sizes and arrays are usually unacceptable.  Use of C++ Class templates is often incompatible with runtime configuration.

If you wish to contribute a pull request then please adopt the above principles or improve upon them.

## Getting started
```unix
git clone https://github.com/tenaciousRas/ledwax-photon.git
cd ledwax-photon
git submodule init
git submodule update
```
NOTE:  The FastLED, LEDMatrix, and LEDSprite libraries are git submodules, hence the git submodule commands above.

### Generate documentation
This project uses Doxygen documentation generator (http://www.doxygen.org).  To view the source-code documentation you must first install DOxygen, then generate the docs.
```unix
cd ledwax-photon
doxygen ledwax_doxygen.config
```

## IDE Setup
The application is configured for two IDEs, AtomIDE and Eclipse.

Eclipse is the primary IDE (supported).  Follow the installation instructions that come with the following software packages:

1. Eclipse Mars C/C++ IDE
2. GNU ARM EABI plugin from https://gnuarmeclipse.github.io/

### Development in Eclipse IDE:
* Download development or stable branch of this repository
* (Using C/C++ (CDT) Perspective in Eclipse Mars):
* Right-click Project Explorer
  * Choose New -> Project
  * Expand C/C++
  * Choose Makefile Project with Existing Code -- click Next
  * Name the project
  * Browse to the folder contianing this project
  * Choose "Cross ARM GCC" as the compiler type (GNU ARM EABI plugin)
  * Complete the Wizard
- Once the project is imported, set the PATH to your Spark Firmware folder:
  * Right-click on the project, choose Properties (at bottom).
  * Expand C/C++
  * Choose Build Variables
  * Choose PARTICLE_FW_PATH - click the Edit button (on right)
  * Enter path to your spark-firmware directory, no trailing slash.
  * clean/build from Eclipse

NOTE:  Compilation results in Eclipse should match those from the command-line (CLI).  If something is wrong with the Eclipse build, try getting the build to run from CLI, then match the environment and command setup in Eclipse.  Eclipse should run (practically) the exact same command as a working "make" command from the CLI.  It may help to **view the Eclipse Console's "CDT Global Build Console"** when troubleshooting the "make" command(s) generated by Eclipse.

### Development in Atom IDE
Atom is the secondary (unsupported) IDE:
1. .atom-build.json is provided to assist in configuring build shell script

#### TODO
- Example circuit with example sketch config

####Credits
Thanks to all the contributors who made this software possible.  This is an evolved codebase.

FastLED SPI library.
LEDMatrix and LEDSprite libraries.
Adafruit (http://www.adafruit.com) for help with I2C library.
Flashee EEPROM library.

Thanks to Sparkfun, Inc. and Adafruit for open source products.

Based on the open-source ledstrip-home Arduino sketch (https://github.com/tenaciousRas/ledstrip-home/).

[particle-funcs]: https://docs.particle.io/reference/api/#call-a-function "particle functions"
[particle-vars]: https://docs.particle.io/reference/api/#get-a-variable-value "particle variables"
