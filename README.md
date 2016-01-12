#LEDWax for Particle Photon

##Cloudy LEDs

An IoT LED controller for Particle Photon with support for PWM LEDs and WS28xx LED Strips.

## Features
* Set LED colors.  Millions of choices.
* Set LED brightness.  255 choices.
* Many display modes for single and multi-pixel LED strips.
* Set color hold time interval.
* Set color fade time interval.
* Native color fading.
* IoT Enabled.
* Supports Particle Photon.
* Support for single-color PWM LED strips.
* Support for multi-color (RGB+, PWM) LED strips.
* Support for WS28xx (SPI) Addressable LED strips.

## Setup

1) Connect LEDs or LED strips to a Particle Photon.
2) Configure the software and flash it to the photon.
3) Control the LED strip using the LEDWax REST API

NOTE: Sparkfun WS2801 strip support is restored but untested.

### Software Configuration
Edit the code in application.cpp.  For example, where we have two strips, one PWM and another WS2812-type (2 strips):
```
	#define NUM_STRIPS 2
	uint8_t STRIP_TYPES[NUM_STRIPS] = { STRIP_TYPE_PWM, STRIP_TYPE_WS2812 };
	uint8_t NUM_LEDS[NUM_STRIPS] = { 1, 60 };
	uint8_t NUM_COLORS_PER_PIXEL[NUM_STRIPS] = { NUM_LEDS_PWM_RGB_STRIP, 3 };
	uint8_t PWM_STRIP_PINS[NUM_STRIPS][3] = { { 0, 1, 2 }, { } };
```

# Usage
Setup LEDWax Photon with a LED strip and a Particle Photon.  Then control the strip using the exposed REST API.

There are only 2 or 3 basic concepts you need to understand for controlling the strips.  First, there is a "mod" command that tells LEDWax how many colors to display and how to cycle through them.  Sending a valid "mod" command will cause the value of the "ledDisplayMode" particle variable to be updated.

Several display modes are supported.  Their behavior varies but there are two (2) basic families of display modes.  In "family 1" the user can configure up to 3 different colors which can be displayed in different ways on the strip.  In "family 2" the colors are pre-set, and the user may have the option to customize a parameter of the display mode (for example:  cycle through a rainbow of pre-defined colors, user-defined length of time to cycle). 

If the displayMode parameter is set to a "family 1" mode, then colors can be customized using the "col" command.

If the displayMode parameter is set to a "family 2" mode, then the multiColorHoldTime and ledFadeTime may or may not have an impact on the display behavior, depending on the mode.

Understanding the above concepts will allow for basic control using raw commands.  There are more commands and variables for finer control and customization.

# LEDWax IoT REST API
##Variables
The folowing particle cloud variables are exposed:
* "numStrips": "int32" - number of LED strips configured in firmware
* "stripIndex": "int32" - current strip being controlled 
* "stripType": "int32" - type of (current) strip
* "dispMode": "int32" - display mode of (current) strip
* "modeColor": "string" - colors assigned to current mode of (current) strip
* "modeColorIdx": "int32" - current color being displayed of (current) strip
* "brightness": "int32" - brightness of (current) strip
* "fadeMode": "int32" - color transition mode of (current) strip
* "fadeTime": "int32" - time spent on color transitions of (current) strip
* "colorTime": "int32" - time spent displaying each color assigned to color mode

##Functions
LEDWAX Photon exposes the following particle cloud functions:
* setLEDParams(String command) : send command to LED strip

#### setLEDParams(String command)
The format for "command" is:
> [command-name];[cmd-value]?[,cmd-value]*

There is no space between the command-name and cmd-value(s).  All commands require a cmd-value - except "qry".  Some commands accept more than one cmd-value, such as "col" (color).  Multiple cmd-values are comma-separated.  Commands are terminated with C++ string termination (\0).

"command-name" can be one of the following:
>	qry : TBD

>	idx : set LED command index -- all following commands will be executed against this LED strip.  Min value is 0, max value is NUM_STRIPS - 1.  Default is 0.

>	col : set LED pixel color.  cmd-value must ahere to the following format:

>		[mode-color-index],[decimal-value-0-to-255]
>	where mode-color-index is the index of the mode color (family 1 display mode) to set

>	brt : set strip brightness. Valid values are from 0 (0% = full off) to 255 (100% = full on).  Brightness is stored sepearately from color in firmware.

>	mod : set strip display mode.  Valid values are:

>		0. solid color (default)
>		1. fade terawatt industries colors
>		2. fade random colors
>		10. fade two colors
>		11. fade three colors
>		12. two alternating colors
>		13. terawatt industries alternating colors
>		14. three alternating colors
>		15. two random alternating colors
>		20. rainbow
>		21. rainbow cycle
>		22. random candy
>		30. cylon

>	mht : Set multi-color-hold-time.  The multi-color-hold-time determines how long each color is displayed before the transition to the next color.  Valid values are [[0-65535] (16-bit integer).

>	lfm : Set LED-fade-mode.  The LED-fade-mode is the style of transition for certain display modes.  A value of 0 disables the fade color transition, so colors switch immediately.  A value of 1 enables the fade transition.  This only applies to display modes 0 - 10.

>	lfi : [TODO]

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

# Hardware
Setting up a circuit with some LEDs isn't too difficult.  If you've never setup a LED circuit from scratch then checkout the fun products at Adafruit and Sparkfun for help.  If time permits an example circuit diagram and sketch configuration will be provided.

The latest version of this firmware is intended for a PWM driver IC to extend the PWM output capabilities of the Photon.  The PWM driver should connect to the I2C interface of the photon.

This project is intended for serious amateur usage.  The user of this software assumes all resposibility.  By using this software you agree to its terms and conditions.  See LICENSE.txt.

I've designed a custom PCB which is undergoing testing so this can be assembled more easily.  The PCB makes it easier to setup different types of LED strips with a Photon.
***
# Develop & Contribute
LEDWax-Photon is a C/C++ project targeted at ARM GNU EABI cross-tools compiling, specifically for the Photon/STM platform.  To compile this software from scratch you need a cross-compiler toolchain, such as the ones found at https://launchpad.net/gcc-arm-embedded/.

The code started out as standard C compiled against g++, but it's been refactored for c++.  There are multiple branches.  The "master" branch is considered stable, but it's currently just a stub.  As of the version 0.1, no branch is considered stable.

## Getting started
```
git clone https://github.com/tenaciousRas/ledwax-photon.git
cd ledwax-photon
```

### Generate documentation
This project uses Doxygen documentation generator (http://www.doxygen.org).  To view the source-code documentation you must first install DOxygen, then generate the docs.
```
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
Thanks to all the contributors who made this software possible.

Adafruit (http://www.adafruit.com) for help with I2C and NeoPixel libraries.

Based on the open-source ledstrip-home Arduino sketch (https://github.com/tenaciousRas/ledstrip-home/).
