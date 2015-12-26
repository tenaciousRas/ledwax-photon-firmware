#LEDWax for Particle Photon

##LEDs on the Cloud

An IoT LED controller with support for PWM LEDs and WS28xx LED Strips.

## Features
- Set LED colors.  Millions of choices.
- Set LED brightness.  255 choices.
- Many display modes for single and multi-pixel LED strips.
- Set color hold time interval.
- Set color fade time interval.
- Native color fading.
- IoT Enabled.
- Supports Particle Photon.
- Support for single-color PWM LED strips.
- Support for multi-color (RGB+, PWM) LED strips.
- Support for WS28xx (SPI) Addressable LED strips.

## Setup

1) Connect LEDs or LED strips to a Particle Photon.
2) Configure the software and flash it to the photon.

NOTE: We lost WS2801 support along the way and hope to restore it.  WS2811/12 support is still here.

### Software Configuration
Edit the code in application.cpp.  For example, where we have two strips, one PWM and another WS2812-type (2 strips):
    #defin NUM_STRIPS 2
	uint8_t STRIP_TYPES[NUM_STRIPS] = { STRIP_TYPE_PWM, STRIP_TYPE_WS2812 };
	uint8_t NUM_LEDS[NUM_STRIPS] = { 1, 60 };
	uint8_t NUM_COLORS_PER_PIXEL[NUM_STRIPS] = { NUM_LEDS_PWM_RGB_STRIP, 3 };
	uint8_t PWM_STRIP_PINS[NUM_STRIPS][3] = { { 0, 1, 2 }, { } };

# Usage
Set LED colors, modes, brightness, and animation using the Particle Cloud.

# IoT API
The folowing particle cloud variables are exposed:
	numStrips : number of LED strips configured in firmware
	stripStateJSON : current state of all led strips

LEDWAX Photon exposes the following particle cloud functions:
	setLEDParams(String command) : send command to LED strip

The format for "command" is:
[command-name][value][,value]*

All commands require a value - except "qry".  There is no space between the command-name and value(s).  Some commands accept more than one value, such as "col" (color).  Multiple values are comma-separated.

The "command" parameter can be one of these:
	qry : refresh stripStateJSON variable
	idx : set LED command index -- all following commands will be executed against this LED strip.  Min value is 0, max value is NUM_STRIPS - 1.  Default is 0.
	col : set LED pixel color.  Format [pixel-index],[decimal-value-0-to-255]
	brt : set strip brightness.  Brightness stored sepearately from color in firmware.
	mod : set strip display mode.  possible values are:
		0: solid color (default)
		1: fade terawatt industries colors
		2: fade random colors
		10: fade two colors
		11: fade three colors
		12: two alternating colors
		13: terawatt alternating colors
		14: three alternating colors
		15: two random alternating colors
		20: rainbow
		21: rainbow cycle
		22: random candy
		30: cylon
	mht : Set multi-color-hold-time.  The multi-color-hold-time determines how long each color is displayed before the transition to the next color.
	lfm : Set LED-fade-mode.  The LED-fade-mode is the style of transition for certain display modes.  A value of 0 will disable the fade color transition, so colors will switch immediately.  A value of 1 enables the fade transition.  This only applies to display modes 0 - 10.
	lfi : [TODO]

# Hardware
Setting up a circuit with some LEDs isn't too difficuly.  If you've never setup a LED circuit from scratch then checkout the fun products at Adafruit and Sparkfun that can help.  If time permits an example circuit diagram and sketch configuration will be provided.

The latest version of this firmware is intended for a PWM driver IC to extend the PWM output capabilities of the Photon.  The PWM driver should connect to the I2C interface of the photon.

This project is intended for serious amateur usage.  The user of this software assumes all resposibility.  By using this software you agree to its terms and conditions.  See LICENSE.txt.

I've designed a custom PCB which is undergoing testing so this can be assembled more easily.  The PCB makes it easier to setup different types of LED strips with a Photon.
***
## Develop & Contribute
LEDWax-Photon is a C/C++ project targeted at ARM GNU EABI cross-tools compiling, specifically for the Photon/STM platform.  The code started out as standard C compiled agaist g++, but it's been refactored for c++.

There are multiple branches.  As of the version 0.1, no branch is considered stable so none are merged to master yet.

The application is configured for two IDEs, AtomIDE and Eclipse.

Eclipse is the primary IDE (supported):
- Eclipse Mars C/C++ IDE
- GNU ARM EABI plugin from https://gnuarmeclipse.github.io/

### Development in Eclipse IDE:
- Download development or stable branch of this repository
- (Using C/C++ (CDT) Perspective in Eclipse Mars):
- Right-click Project Explorer
-- Choose New -> Project
-- Expand C/C++
-- Choose Makefile Project with Existing Code -- click Next
-- Name the project
-- Browse to the folder contianing this project
-- Choose "Cross ARM GCC" as the compiler type (GNU ARM EABI plugin)
-- Complete the Wizard
- Once the project is imported, set the PATH to your Spark Firmware folder:
-- Right-click on the project, choose Properties (at bottom).
-- Expand C/C++
-- Choose Build Variables
-- Choose PARTICLE_FW_PATH - click the Edit button (on right)
-- Enter path to your spark-firmware directory, no trailing slash.
-- Projct -> C/C++ Index -> Rebuild
-- clean/build from Eclipse

### Development in Atom IDE
Atom is the secondary (unsupported) IDE:
- .atom-build.json is provided to assist in configuring build shell script

#### TODO
- Example circuit with example sketch config

####Credits
Thanks to all the contributors who made this software possible.

Adafruit (http://www.adafruit.com) for help with I2C and NeoPixel libraries.

Based on the open-source ledstrip-home Arduino sketch (https://github.com/tenaciousRas/ledstrip-home/).
