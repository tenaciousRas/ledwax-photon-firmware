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
#include "ledwax_photon.h"

#define _DEBUG_MODE 1

// LED Strip setup
// set number of LED strips
#define NUM_STRIPS 2

using namespace ledwax;

uint8_t STRIP_TYPES[NUM_STRIPS] = { STRIP_TYPE_PWM, STRIP_TYPE_WS2812 };
uint8_t NUM_LEDS[NUM_STRIPS] = { 1,
NUM_LEDS_SPARKFUN_WS2801_1METER };
uint8_t NUM_COLORS_PER_PIXEL[NUM_STRIPS] = { NUM_LEDS_PWM_RGB_STRIP, 3 };
uint8_t PWM_STRIP_PINS[NUM_STRIPS][3] = { { 0, 1, 2 }, { } };

LEDWaxPhoton LedWax = LEDWaxPhoton(
        STRIP_TYPES, NUM_LEDS, NUM_COLORS_PER_PIXEL, PWM_STRIP_PINS);

int setLEDParams(String);

void setup() {
#ifdef _DEBUG_MODE
#endif
    LedWax.begin();
    // set particle functions
    Particle.function(
            "setLEDParams", &setLEDParams);
    // set vars
    Particle.variable(
            "numStrips", &LedWax.numStrips, INT);
    Particle.variable(
            "stripStateJSON", &LedWax.stripStateJSON, STRING);
#ifdef _DEBUG_MODE
#endif
}

void loop() {
    LedWax.renderAll();
}

int setLEDParams(String command) {
    bool validCommand = false;
    string cmd = string(
            command);
    uint16_t cmdLen = cmd.length();
    uint8_t howManyParams = 0;
    uint16_t cmdDelimPos = 0;
    for (int i = 0; i < cmdLen; i++) {
        cmdDelimPos = cmd.find(
                ";", cmdDelimPos);
        if (cmdDelimPos != std::string::npos) {
            howManyParams++;
        }
    }
    string parsedCmds[howManyParams];
    char* cmdPart = strtok(
            strdup(cmd.c_str()), ",");
    parsedCmds[0] = string(
            cmdPart);
    for (int i = 1; i < howManyParams; i++) {
        cmdPart = strtok(
                NULL, ",");
    }
    if (parsedCmds[0] == "qry") {
        // FIXME move to ledwax
        LedWax.stripStateJSON = LedWax.buildStripStateJSON().c_str();
        // DONE FIXME
        Particle.publish(
                "ledStripDisplayState", LedWax.stripStateJSON);
    } else if (parsedCmds[0] == "idx") {
        LedWax.setRemoteControlStripIndex(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "col") {
        LedWax.setLEDStripColor(
                parsedCmds[1] + "," + parsedCmds[1]);
    } else if (parsedCmds[0] == "brt") {
        LedWax.setBright(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "mod") {
        LedWax.setDispMode(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "mht") {
        LedWax.setMultiColorHoldTime(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "lfm") {
        LedWax.setLedFadeMode(
                parsedCmds[1]);
    } else {
        // invalid command
        validCommand = false;
    }
    if (!validCommand) {
        return 1;
    }
    return 0;
}

