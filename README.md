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
- Support for multi-color (RGB+) LED strips.
- Support for WS28xx (SPI) Addressable LED strips.

## Setup

1) Connect LEDs or LED strips to a Particle Photon.
2) Configure the software and flash it to the photon.

Then set LED colors, modes, brightness, and animation using the Particle Cloud.

Setting up a circuit with some LEDs isn't too difficult.  If you've never setup a LED circuit from scratch then checkout the fun products at Adafruit and Sparkfun that can help.  If time permits an example circuit diagram and sketch configuration will be provided.

** NOTE: Read the comments in application.cpp for explanations how to configure the code to match your hardware.

This project is intended for serious amateur usage.  I've designed a custom PCB which is undergoing testing so this can be assembled more easily.  The PCB makes it easier to setup different types of LED strips with a Photon.

## Contribute
LEDWax-Photon is a C/C++ project targeted at ARM GNU EABI cross-tools compiling, specifically for the Photon/STM24 platform.

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
-- clean/build from Eclipse

### Development in Atom IDE
Atom is the secondary (unsupported) IDE:
- .atom-build.json is provided to assist in configuring build shell script

#### TODO
- Example circuit with example sketch config

####Credits
Thanks to all the contributors who made this software possible.

Adafruit (http://www.adafruit.com) for help with I2C and WS2801 libraries.

Based on the open-source ledstrip-home Arduino sketch (https://github.com/tenaciousRas/ledstrip-home/).
