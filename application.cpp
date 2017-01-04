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

using namespace ledwax;

// function prototypes
int setLEDParams(String);
int resetAllStripsToDefault(String);
void refreshParticleVars();
static inline ledwaxconfig::LEDWaxConfig*& init_strips();

// LED Strip setup.
// *********** EDIT THIS SECTION ACCORDING TO HARDWARE ***********
#define NUM_STRIPS 3
// *********** END EDIT THIS SECTION ***********

static inline ledwaxconfig::LEDWaxConfig*& init_strips() {
    static ledwaxconfig::LEDWaxConfig* config = new ledwaxconfig::LEDWaxConfig[NUM_STRIPS];
    // *********** EDIT THIS SECTION ACCORDING TO HARDWARE ***********
    config[0].setStripType(STRIP_TYPE_WS2811);
    config[0].setStripNumPixels(60);
    config[0].setNumColorsPerPixel(3);
    config[0].setSpiPins(new uint8_t[WIRE_NUM_SPI_1_WIRE]);
    config[0].getSpiPins()[0] = A5;
    config[0].setMatrix(true);
    config[0].setMatrixHeight(8);

    config[1].setStripType(STRIP_TYPE_I2C_PWM);
    config[1].setStripNumPixels(1);
    config[1].setNumColorsPerPixel(NUM_PIXELS_PER_LED_PWM_RGB_STRIP);
    config[1].setI2cAddy(0xE0);
    config[1].setI2cPwmPins(new uint8_t[NUM_PIXELS_PER_LED_PWM_RGB_STRIP]);
    config[1].getI2cPwmPins()[0] = 0;
    config[1].getI2cPwmPins()[0] = 1;
    config[1].getI2cPwmPins()[0] = 2;
    config[1].setMatrix(false);

    config[2].setStripType(STRIP_TYPE_NATIVE_PWM);
    config[2].setStripNumPixels(1);
    config[2].setNumColorsPerPixel(NUM_PIXELS_PER_LED_PWM_RGB_STRIP);
    config[2].setNativePwmPins(new uint8_t[NUM_PIXELS_PER_LED_PWM_RGB_STRIP]);
    config[2].getNativePwmPins()[0] = RX;
    config[2].getNativePwmPins()[0] = TX;
    config[2].getNativePwmPins()[0] = WKP;
    config[2].setMatrix(false);
    // *********** END EDIT THIS SECTION ***********
    return config;
}

// global vars
ledwaxconfig::LEDWaxConfig (*STRIP_CONFIGS) = init_strips();
int numStrips = NUM_STRIPS;     //particle var
ledwax::LEDWaxPhoton* LedWax = new LEDWaxPhoton(
        (uint8_t) numStrips, &STRIP_CONFIGS[0]);
// particle vars = state members from LedWaxPhoton::led_strip_disp_state
int remoteControlStripIndex, stripType, dispMode, ledFadeMode, ledModeColorIndex, ledStripBrightness, stripNumPixels, numColorsPerPixel;
long multiColorHoldTime, fadeTimeInterval;
char *ledModeColor = new char[VAR_SZ_LED_MODE_COLOR]; // dereference for particle string var

void setup() {
    LedWax->begin();
    // init watchvars
    // set particle functions
    Particle.function(
            "setLEDParams", &setLEDParams);
    Particle.function(
            "resetAll", &resetAllStripsToDefault);
    // set particle vars - copy from object to global state
    refreshParticleVars();
    Particle.variable(
            "numStrips", &numStrips, INT);
    Particle.variable(
            "stripIndex", &remoteControlStripIndex, INT);
    Particle.variable(
            "stripType", &stripType, INT);
    Particle.variable(
            "numPixels", &stripNumPixels, INT);
    Particle.variable(
            "numPixColors", &numColorsPerPixel, INT);
    Particle.variable(
            "dispMode", &dispMode, INT);
    Particle.variable(
            "modeColor", ledModeColor, STRING);
    Particle.variable(
            "modeColorIdx", &ledModeColorIndex, INT);
    Particle.variable(
            "brightness", &ledStripBrightness, INT);
    Particle.variable(
            "fadeMode", &ledFadeMode, INT);
    Particle.variable(
            "fadeTime", &fadeTimeInterval, INT);
    Particle.variable(
            "colorTime", &multiColorHoldTime, INT);
#ifdef __LWAX_PHOTON_DEBUG_MODE
#endif
}

void loop() {
    LedWax->renderStrips();
    refreshParticleVars();
}

/**
 * Refresh particle vars.  Copy values from {this#LedWax} to global state.
 */
void refreshParticleVars() {
    numStrips = (int) LedWax->numStrips;
#ifdef __LWAX_PHOTON_DEBUG_MODE
//    numStrips = *((uint8_t *) LedWax->stripPins + (1 * 3 + 0));
//    numStrips = *(STRIP_PINS + 3);
//    numStrips = pinDefs[1][0];
//    numStrips = *(*(pinDefs + 1) + 0);    // compiler knows array column width
//    numStrips = (int) LedWax->stripNumPixels[0];
#endif
    remoteControlStripIndex = LedWax->remoteControlStripIndex;
    stripType = LedWax->stripConfigs[remoteControlStripIndex].getStripType();
    stripNumPixels = LedWax->stripConfigs[remoteControlStripIndex].getStripNumPixels();
    numColorsPerPixel = LedWax->stripConfigs[remoteControlStripIndex].getNumColorsPerPixel();
    dispMode = LedWax->stripState[remoteControlStripIndex].dispMode;
    ledFadeMode = LedWax->stripState[remoteControlStripIndex].ledFadeMode;
    LedWax->buildLedModeColorJSONArr(
            LedWax->remoteControlStripIndex);
    memcpy(ledModeColor, LedWax->ledModeColorJSONArr, VAR_SZ_LED_MODE_COLOR);
    ledModeColorIndex = LedWax->stripState[remoteControlStripIndex].ledModeColorIndex;
    multiColorHoldTime = LedWax->stripState[remoteControlStripIndex].multiColorHoldTime;
    fadeTimeInterval = LedWax->stripState[remoteControlStripIndex].fadeTimeInterval;
    ledStripBrightness = (int) (LedWax->stripState[LedWax->remoteControlStripIndex].ledStripBrightnessScale);
}

/**
 * Handler for setLedParams() Particle function.
 */
int setLEDParams(String command) {
    bool validCommand = true;
    string cmd = command.c_str();
    uint16_t cmdLen = cmd.length();
    uint8_t howManyParams = 0;
    uint16_t cmdDelimPos = 0;
    for (int i = 0; i < cmdLen; i++) {
        cmdDelimPos = cmd.find(
                ";", cmdDelimPos);
        if (cmdDelimPos != std::string::npos) {
            howManyParams++;
        } else {
            if (cmdDelimPos < cmdLen - 1) {
                howManyParams++;
            }
            break;
        }
    }
// TODO combine with above loop
    string parsedCmds[howManyParams];
    char* cmdPart = strtok(
            strdup(cmd.c_str()), ";");
    parsedCmds[0] = string(
            cmdPart);
    for (int i = 1; i < howManyParams; i++) {
        cmdPart = strtok(
                NULL, ";");
        parsedCmds[i] = cmdPart;
    }
    if (parsedCmds[0] == "idx") {
        LedWax->setRemoteControlStripIndex(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "col") {
        LedWax->setModeLEDColor(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "brt") {
        LedWax->setBright(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "mod") {
        LedWax->setDispMode(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "mht") {
        LedWax->setMultiColorHoldTime(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "lfm") {
        LedWax->setLedFadeMode(
                parsedCmds[1]);
    } else if (parsedCmds[0] == "lfti") {
        LedWax->setLedFadeTimeInterval(
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

/**
 * Handler for setLedParams() Particle function.
 */
int resetAllStripsToDefault(String command) {
    LedWax->resetAllStripsToDefault();
    return 0;
}
