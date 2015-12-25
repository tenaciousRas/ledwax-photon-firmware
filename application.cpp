/*
 Created By:
 Free Beachler
 Longevity Software LLC
 Terawatt Industries 2015
 :::
 Conveyed to you under the Apache 2.0 license, because we LOVE open source software and hardware.  Enjoy!
 :::
 Description:
 :::
 This sketch provides a programmable wireless controller for a LED strip(s) using an Arduino and Bluetooth (or other serial).  Commands can be sent to a BT module
 attached to an Arduino to program the mode of the LED strip.  This can be used for home lighting strips, for example.  Programmable feature values are stored in EEPROM
 so the LED strip starts in the last-known-state when it's turned on.

 This code supports multiple types of LED strips.  Initially WS2801,WS2811/12 strips are supported, through support graciously provided by the AdafruitWS2801 library.  Support
 for PWM-based single-color or RGB strips will be added.

 Programmable features include color, brightness, fade mode, animations, and more.

 Different features are available for addressable and non-addresable strips.
 :::
 Sparkfun WS2801 LED Strip :::
 You will need to connect 5V/Gnd from the Arduino (USB power seems to be sufficient).

 For the data pins, please pay attention to the arrow printed on the strip. You will need to connect to
 the end that is the begining of the arrows (data connection)--->

 If you have a 4-pin connection:
 Blue = 5V
 Red = SDI
 Green = CKI
 Black = GND

 If you have a split 5-pin connection:
 2-pin Red+Black = 5V/GND
 Green = CKI
 Red = SDI
 :::
 */
#include "lib/spark-flashee-eeprom/flashee-eeprom.h"
#include "lib/neopixel/neopixel.h"
#include <sstream>
#include <string>
#include "wiring/inc/spark_wiring_cloud.h"
#include "wiring/inc/spark_wiring.h"
#include "ledwax_photon.h"

#define _DEBUG_MODE 1

using namespace std;
using namespace Flashee;

// LED Strip setup
// set number of LED strips
#define NUM_STRIPS 2
// Set number of pixels in an addressable strip. 25 = 25 pixels in a row.
// Does not apply to single-color and non-addressable RGB strips.

// Set number color of PWM STRIPs

LEDWaxPhoton LedWax;

void setup() {
#ifdef _DEBUG_MODE
#endif
	LedWax = LEDWaxPhoton( { STRIP_TYPE_PWM, STRIP_TYPE_WS2812 }, { 1,
	NUM_LEDS_SPARKFUN_WS2801_1METER }, { NUM_LEDS_PWM_RGB_STRIP, 3 }, { { 0, 1,
			2 }, { } });
	LedWax.begin();
	// set particle functions
	Particle.function("setLEDParams", &setLEDParams);
	// set vars
	Particle.variable("numStrips", &LedWax.numStrips, INT);
	Particle.variable("stripStateJSON", &LedWax.stripStateJSON, STRING);
#ifdef _DEBUG_MODE
#endif
}

void loop() {
	LedWax.renderAll();
}

int setLEDParams(string command) {
	bool validCommand = false;
	uint16_t cmdLen = command.length();
	uint8_t howManyParams = 0;
	int cmdDelimPos = 0;
	for (int i = 0; i < cmdLen; i++) {
		cmdDelimPos = command.find(";", cmdDelimPos);
		if (cmdDelimPos != std::string::npos) {
			howManyParams++;
		}
	}
	string parsedCmds[howManyParams];
	char* cmdPart = strtok(strdup(command.c_str()), ",");
	parsedCmds[0] = string(cmdPart);
	for (int i = 1; i < howManyParams; i++) {
		cmdPart = strtok(NULL, ",");
	}
	if (parsedCmds[0] == "qry") {
	} else if (parsedCmds[0] == "idx") {
	} else if (parsedCmds[0] == "col") {
	} else if (parsedCmds[0] == "brt") {
	} else if (parsedCmds[0] == "mod") {
	} else if (parsedCmds[0] == "mht") {
	} else if (parsedCmds[0] == "lfm") {
	} else {
		// invalid command
	}
}

