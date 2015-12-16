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
//#include <SPI.h>
#include "lib/spark-flashee-eeprom/flashee-eeprom.h"
#include "lib/neopixel/neopixel.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include "wiring/inc/spark_wiring_cloud.h"
#include "wiring/inc/spark_wiring.h"
#include "application.h"

using namespace std;
using namespace Flashee;

#define _VERSION 5  // it's probably more like v.100
#define DEBUG_MODE 1

#define STRIP_TYPE_WS2801 1
#define STRIP_TYPE_WS2811 2  // TODO support
#define STRIP_TYPE_WS2812 3  // TODO support
#define STRIP_TYPE_PWM  10

// LED Strip setup
// set number of LED strips
#define NUM_STRIPS 2
// Set number of pixels in an addressable strip. 25 = 25 pixels in a row.
// Does not apply to single-color and non-addressable RGB strips.
#define NUM_LEDS_SPARKFUN_WS2801_1METER 10
// Set number color of PWM STRIPs
#define NUM_LEDS_PWM_WHITE_STRIP  1  // single color PWM
#define NUM_LEDS_PWM_RGB_STRIP  3  // RGB PWM

uint8_t stripType[NUM_STRIPS] = { STRIP_TYPE_PWM, STRIP_TYPE_WS2801 };
uint8_t stripNumPixels[NUM_STRIPS] = { 1, NUM_LEDS_SPARKFUN_WS2801_1METER }; // 1 for PWM strip
uint8_t stripNumColorsPerPixel[NUM_STRIPS] = { NUM_LEDS_PWM_RGB_STRIP, 3 };
// Initialize strip variables.  Interesting C implementation.  Define two arrays, one for
// addressable strips, one for PWM.  Effectively define position of strips by populating specific members
// of each array.
// FIXME improve implementation
Adafruit_NeoPixel addressableStrip1 = Adafruit_NeoPixel(
NUM_LEDS_SPARKFUN_WS2801_1METER);
Adafruit_NeoPixel* addressableStrips[NUM_STRIPS] = { NULL, &addressableStrip1 };
uint8_t pwmStripPins[NUM_STRIPS][NUM_LEDS_PWM_RGB_STRIP] = { { 5, 9, 3 }, { } };

#define OFF 0x000000
#define RED 0xFF0000
#define BLUE 0x0000FF
#define GREEN 0x00FF00
#define YELLOW 0xFFFF00
#define CYAN 0x00FFFF
#define VIOLET 0xFF00FF
#define WHITE 0xFFFFFF
#define DIM_RED 0x330000
#define DIM_BLUE 0x000033
#define DIM_GREEN 0x003300
#define DIM_YELLOW 0x333300
#define DIM_CYAN 0x003333
#define DIM_VIOLET 0x330033
#define DIM_WHITE 0x333333
#define TWYELLOW 0xFFEE00
#define TWBLUE 0x00A5D5

/**
 * display mode values:
 * 0: solid color
 * 1: fade terawatt colors
 * 2: fade random colors
 * 10: fade two colors
 * 11: fade three colors
 * 12: two alternating colors
 * 13: terawatt alternating colors
 * 14: three alternating colors
 * 15: two random alternating colors
 * 20: rainbow
 * 21: rainbow cycle
 * 22: random candy
 * 30: cylon
 */
#define DEFAULT_MULTI_COLOR_HOLD_TIME 5000;  // time to hold colors when showing multiple colors
#define DEFAULT_DISP_MODE 20;
#define DEFAULT_LED_FADE_MODE 2;
#define INITIAL_MULTI_COLOR_ALT_STATE 0;
#define DEFAULT_LED_STRIP_BRIGHTNESS 1.0;

led_strip_disp_state stripState[NUM_STRIPS];
uint8_t remoteControlStripIndex;

unsigned long multiColorNextColorTime[NUM_STRIPS];
unsigned long ledColor[NUM_STRIPS][NUM_LEDS_SPARKFUN_WS2801_1METER] = { { RED,
OFF }, { TWYELLOW, TWBLUE }
//  {RED, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF},
//  {TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE}
		};

#define LED_FADE_STEPS 16;
#define LED_FADE_STEP_DELAY_MS 50;  // microsecs between fade steps
uint32_t ledColorOld[NUM_STRIPS][NUM_LEDS_SPARKFUN_WS2801_1METER] = {
		{ RED, OFF }, { TWYELLOW, TWBLUE } };  // color before fade
uint32_t ledColorFadeTo[NUM_STRIPS][NUM_LEDS_SPARKFUN_WS2801_1METER] = { { RED,
OFF }, { TWYELLOW, TWBLUE } };    // fade-to color
unsigned long ledFadeStepTime[NUM_STRIPS];  // time to next fade step
int ledFadeStepIndex[NUM_STRIPS];  // color distance divided by LED_FADE_STEPS
double ledFadeStep[NUM_STRIPS][NUM_LEDS_SPARKFUN_WS2801_1METER][3]; // 3 for each RGB component
uint8_t rainbowStepIndex[NUM_STRIPS] = { 0, 0 };

FlashDevice* flash;
int eepromAddyStripState = 4;  // eeprom addy to store strip state

int numStrips = NUM_STRIPS;
String stripStateJSON;

void setup() {
	// set the data rate for bt device
//  //Serial.begin(9600);
#ifdef DEBUG_MODE
//  //Serial.print(F("startup\n"));
#endif
	// init eeprom
	flash = Devices::createDefaultStore();
	// set callback functions
	Particle.function("setLEDParams", setLEDParams);
	// set vars
	Particle.variable("numStrips", &numStrips, INT);
	Particle.variable("stripStateJSON", &stripStateJSON, STRING);
#ifdef DEBUG_MODE
//  //Serial.print(F("configured bt\n"));
#endif
	remoteControlStripIndex = 0;
	for (int i = 0; i < numStrips; i++) {
		initStripState(i);
		// read eeprom
		readStripState(&stripState[i]);
		// setup LEDs
		setDispModeColors(i, stripState[i].dispMode);
	}
	addressableStrips[1]->begin();
	addressableStrips[1]->show();
}

void loop() {
	for (int i = 0; i < numStrips; i++) {
		if (stripState[i].fading) {
			doFade(i);
			renderPixels(i);
			continue;
		}
		switch (stripState[i].dispMode) {
		case 0:
			solidOneColor(i);
			break;
		case 1:
			solidTwoColors(i);
			break;
		case 2:
			solidTwoColors(i);
			break;
		case 10:
			solidTwoColors(i);
			break;
		case 11:
			solidThreeColors(i);
			break;
		case 12:
			alternatingTwoColors(i);
			break;
		case 13:
			alternatingTwoColors(i);
			break;
		case 14:
			alternatingThreeColors(i);
			break;
		case 15:
			alternatingTwoRandomColors(i);
			break;
		case 20:
			rainbow(i, 1000);
			break;
		case 21:
			rainbowCycle(i, 1000);
			break;
		case 22:
			randomCandy(i);
			break;
		case 30:
			break;
		default:
			break;
		}
	}
}

void initStripState(uint8_t stripNum) {
	stripState[stripNum].fading = false;
	stripState[stripNum].dispMode = DEFAULT_DISP_MODE
	;
	stripState[stripNum].ledFadeMode = DEFAULT_LED_FADE_MODE
	;
	stripState[stripNum].multiColorAltState = 0;
	stripState[stripNum].ledModeColor[0] = TWYELLOW;
	stripState[stripNum].ledModeColor[1] = TWBLUE;
	stripState[stripNum].ledModeColor[2] = OFF;
	stripState[stripNum].multiColorHoldTime = DEFAULT_MULTI_COLOR_HOLD_TIME
	;
	stripState[stripNum].fadeTimeInterval = LED_FADE_STEP_DELAY_MS
	;
	stripState[stripNum].ledStripBrightness = DEFAULT_LED_STRIP_BRIGHTNESS
	;
}

void readStripState(led_strip_disp_state* ret) {
	unsigned int offset = eepromAddyStripState + sizeof(int); // store int at addy 0
	uint8_t bData;
	ret->fading = false;
	ret->multiColorAltState = INITIAL_MULTI_COLOR_ALT_STATE
	;
	flash->read(bData, offset++);
	ret->dispMode = (uint8_t) bData;
	flash->read(bData, offset++);
	ret->ledFadeMode = (uint8_t) bData;
	unsigned long color;
	for (int i = 0; i < 3; i++) {
		color = 0;
		for (int j = 3; j >= 0; j--) {
			flash->read(bData, offset++);
			color += (bData << (8 * j));
		}
		ret->ledModeColor[i] = color;
	}
	unsigned long holdTime;
	holdTime = 0;
	for (int j = 3; j >= 0; j--) {
		flash->read(bData, offset++);
		holdTime += (bData << (8 * j));
	}
	ret->multiColorHoldTime = holdTime;
	unsigned int storedLedStripBrightness;
	storedLedStripBrightness = 0;
	for (int j = 1; j >= 0; j--) {
		flash->read(bData, offset++);
		storedLedStripBrightness += (bData << (8 * j));
	}
	ret->ledStripBrightness = (float) storedLedStripBrightness / (float) 1024;
#ifdef DEBUG_MODE
//  //Serial.print(F("multiColorHoldTime = "));
//  //Serial.print((ret->multiColorHoldTime));
//  //Serial.print(F("\nread eeprom done\n"));
#endif
}

void putStripState(led_strip_disp_state* lsds) {
	led_strip_disp_state storedState;
	readStripState(&storedState);
	unsigned int offset = eepromAddyStripState + sizeof(int); // store int at addy 0
	if ((unsigned int) storedState.dispMode != (unsigned int) lsds->dispMode) {
		// store byte
		flash->write((uint8_t) lsds->dispMode, offset++);
	}
	offset++;
	if (storedState.ledFadeMode != lsds->ledFadeMode) {
		// store byte
		flash->write((uint8_t) lsds->ledFadeMode, offset++);
	}
	offset++;
	for (int i = 0; i < 3; i++) {
		if (storedState.ledModeColor[i] != lsds->ledModeColor[i]) {
			// store long
			for (int j = 3; j >= 0; j--) {
				flash->write(((uint8_t) ((lsds->ledModeColor[i] >> (8 * j)) & 0xFF)), offset++);
			}
		} else {
			offset += 4;
		}
	}
	if (storedState.multiColorHoldTime != lsds->multiColorHoldTime) {
		// store long
		for (int j = 3; j >= 0; j--) {
			flash->write(((uint8_t) ((lsds->multiColorHoldTime >> (8 * j)) & 0xFF)), offset++);
		}
	} else {
		offset += 4;
	}
	if (storedState.ledStripBrightness != lsds->ledStripBrightness) {
		// store int
		unsigned int lsb = lsds->ledStripBrightness * 1024;
		for (int j = 1; j >= 0; j--) {
			flash->write(((uint8_t) ((lsb >> (8 * j)) & 0xFF)), offset++);
		}
	} else {
		offset += 2;
	}
	// update strip state watchvar
	stripStateJSON = buildStripStateJSON().c_str();
	Particle.publish("ledStripDisplayState", stripStateJSON);
#ifdef DEBUG_MODE
//  //Serial.print(F("write eeprom done\n"));
#endif
}

string buildStripStateJSON() {
	ostringstream os;
	os << "{ ledStripDisplayState:[";
	for (int i = 0; i < numStrips; i++) {
		os << "{";
		os << "stripType:" << "'" << stripType[i] << "'";
		os << ",";
		os << "stripNumPixels:" << "'" << stripNumPixels[i] << "'";
		os << ",";
		os << "ledFadeMode:" << "'" << stripState[i].ledFadeMode << "'";
		os << ",";
		os << "ledStripBrightness:" << "'" << stripState[i].ledStripBrightness
				<< "'";
		os << ",";
		os << "ledModeColor:[";
		for (int j = 0; j < stripNumColorsPerPixel[i]; j++) {
			os << stripState[i].ledModeColor[j];
			if (j < (stripNumColorsPerPixel[i] - 1)) {
				os << ",";
			}
		}
		os << "],";
		os << "multiColorAltState:" << "'" << stripState[i].multiColorAltState
				<< "'";
		os << ",";
		os << "multiColorHoldTime:" << "'" << stripState[i].multiColorHoldTime
				<< "'";
		os << ",";
		os << "fadeTimeInterval:" << "'" << stripState[i].fadeTimeInterval
				<< "'";
		os << "}";
		if (i < numStrips - 1) {
			os << ",";
		}
	}
	os << "] }";
	return os.str();
}

void setDispModeColors(uint8_t stripNum, int mode) {
	switch (mode) {
	case 0:
		break;
	case 1:
		stripState[stripNum].ledModeColor[0] = TWYELLOW;
		stripState[stripNum].ledModeColor[1] = TWBLUE;
		break;
	case 2:
		stripState[stripNum].ledModeColor[0] = rand()  & 0xFFFFFF;
		stripState[stripNum].ledModeColor[1] = rand()  & 0xFFFFFF;
		break;
	case 10:
		break;
	case 11:
		break;
	case 12:
		break;
	case 13:
		stripState[stripNum].ledModeColor[0] = TWYELLOW;
		stripState[stripNum].ledModeColor[1] = TWBLUE;
		break;
	case 14:
		break;
	case 15:
		stripState[stripNum].ledModeColor[0] = rand()  & 0xFFFFFF;
		stripState[stripNum].ledModeColor[1] = rand()  & 0xFFFFFF;
		break;
	case 20:
		rainbowStepIndex[stripNum] = 0;
		break;
	case 21:
		rainbowStepIndex[stripNum] = 0;
		break;
	case 22:
		rainbowStepIndex[stripNum] = 0;
		break;
	case 30:
		break;
	default:
		break;
	}
	putStripState(&stripState[stripNum]);
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

bool startsWith(const char *pre, const char *str) {
	size_t lenpre = strlen(pre), lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

int setRemoteControlStripIndex(string command) {
	int bData;
	bData = atoi(command.c_str());
	if (0 > bData) {
		bData = 0;
	}
	if (bData > numStrips - 1) {
		bData = numStrips - 1;
	}
	remoteControlStripIndex = bData;
#ifdef DEBUG_MODE
//Serial.print(F("remoteControlStripIndex = "));
//Serial.print(remoteControlStripIndex);
#endif
	return 0;
}

int setLEDStripColor(string command) {
	char data[40];
	// expect to always have two Ints from Android app
	uint8_t b1[2];
	uint8_t b2[38];
	b1[0] = command[0];
	b1[1] = '\0';
	int a = 2;
	for (; a < 40; a++) {
		b2[a - 2] = command[a];
		if (b2[a - 2] == 0) {
			break;
		}
	}
	int colorIndex = atoi((char *) b1);
	unsigned long color = atol((char *) b2);
	color = (color << 8) >> 8;
	stripState[remoteControlStripIndex].ledModeColor[colorIndex] = color;
	putStripState(&stripState[remoteControlStripIndex]);
#ifdef DEBUG_MODE
//Serial.print(F("colorIndex = "));
//Serial.print(colorIndex);
//Serial.print(F(","));
//Serial.print(F("color = "));
//Serial.print(color);
//Serial.print(F("\n"));
//Serial.println(data);
//Serial.print(F("\n"));
#endif
	return 0;
}

int setDispMode(string command) {
	int bData;
	bData = atoi(command.c_str());
	stripState[remoteControlStripIndex].dispMode = bData;
	setDispModeColors(remoteControlStripIndex,
			stripState[remoteControlStripIndex].dispMode);
	stripState[remoteControlStripIndex].multiColorAltState =
	INITIAL_MULTI_COLOR_ALT_STATE
	;
	stripState[remoteControlStripIndex].fading = false;
	putStripState(&stripState[remoteControlStripIndex]);
#ifdef DEBUG_MODE
//Serial.print(F("dispMode = "));
//Serial.print((int) bData);
//Serial.print(F(", "));
//Serial.print(stripState[remoteControlStripIndex].dispMode);
//Serial.print(F("\n"));
#endif
	return 0;
}

int setBright(string command) {
	int bData;
	bData = atoi(command.c_str());
	if (255 < bData) {
		bData = 255;
	}
	stripState[remoteControlStripIndex].ledStripBrightness = (float) bData
			/ (float) 255.0;
	putStripState(&stripState[remoteControlStripIndex]);
#ifdef DEBUG_MODE
//Serial.print(F("ledStripBrightness = "));
//Serial.print(bData);
//Serial.print(F(", "));
//Serial.print((float) stripState[remoteControlStripIndex].ledStripBrightness);
//Serial.print(F("\n"));
#endif
	return 0;
}

int setLedFadeTimeInterval(string command) {
	unsigned long bData;
	bData = atol(command.c_str());
	stripState[remoteControlStripIndex].fadeTimeInterval = bData;
	putStripState(&stripState[remoteControlStripIndex]);
	return 0;
}

int setMultiColorHoldTime(string command) {
	unsigned long bData;
	bData = atol(command.c_str());
	stripState[remoteControlStripIndex].multiColorHoldTime = bData;
	putStripState(&stripState[remoteControlStripIndex]);
	return 0;
}

int setLedFadeMode(string command) {
	unsigned int bData;
	bData = atoi(command.c_str());
	if (11 < stripState[remoteControlStripIndex].dispMode) {
		// only for solid colors
		return 1;
	}
	if (1 < bData) {
		bData = 0;
	}
	stripState[remoteControlStripIndex].ledFadeMode = bData;
	putStripState(&stripState[remoteControlStripIndex]);
#ifdef DEBUG_MODE
//Serial.print(F("ledFadeMode = "));
//Serial.print(stripState[remoteControlStripIndex].ledFadeMode);
//Serial.print(F("\n"));
#endif
	return 0;
}

void refreshLEDs(uint8_t stripNum) {
	// TODO implement with realtime fader
}

void turnOffLEDs(uint8_t stripNum) {
	// TODO re-implement
	for (int i = 0; i < stripNumPixels[stripNum]; i++) {
		ledColor[stripNum][i] = OFF;
	}
}

void white(uint8_t stripNum) {
	// TODO re-implement
	for (int i = 0; i < stripNumPixels[stripNum]; i++) {
		ledColor[stripNum][i] = WHITE;
	}
}

void solidMultiColor(uint8_t stripNum, int numMultiColors) {
	if (multiColorNextColorTime[stripNum] - millis()
			> stripState[stripNum].multiColorHoldTime) {
		multiColorNextColorTime[stripNum] = millis()
				+ stripState[stripNum].multiColorHoldTime;
		for (int x = stripNumPixels[stripNum] - 1; x >= 0; x--) {
			ledColorFadeTo[stripNum][x] =
					stripState[stripNum].ledModeColor[stripState[stripNum].multiColorAltState];
		}
#ifdef DEBUG_MODE
//Serial.print(F("multiColorAltState,ledModeColor[multiColorAltState] = "));
//Serial.print(stripState[stripNum].multiColorAltState);
//Serial.print(F(", "));
//Serial.print(stripState[stripNum].ledModeColor[stripState[stripNum].multiColorAltState]);
//Serial.print(F(", multiColorNextTime,currentMillis = "));
//Serial.print(multiColorNextColorTime[stripNum]);
//Serial.print(F(", "));
//Serial.print(millis());
//Serial.print(F("\n"));
#endif
		stripState[stripNum].multiColorAltState =
				(stripState[stripNum].multiColorAltState + 1) % numMultiColors;
		startFade(stripNum);
	}
}

void alternatingMultiColor(uint8_t stripNum, int numMultiColors) {
	for (int x = (stripNumPixels[stripNum] - 1); x >= 0; x -= numMultiColors) {
		for (int y = 0; y < numMultiColors; y++) {
			ledColorFadeTo[stripNum][x - y] =
					stripState[stripNum].ledModeColor[y];
		}
	}
	startFade(stripNum);
}

void solidOneColor(uint8_t stripNum) {
	solidMultiColor(stripNum, 1);
}

void solidTwoColors(uint8_t stripNum) {
	solidMultiColor(stripNum, 2);
}

void solidThreeColors(uint8_t stripNum) {
	solidMultiColor(stripNum, 3);
}

void alternatingTwoColors(uint8_t stripNum) {
	alternatingMultiColor(stripNum, 2);
}

void alternatingTwoRandomColors(uint8_t stripNum) {
	alternatingMultiColor(stripNum, 2);
}

void alternatingThreeColors(uint8_t stripNum) {
	alternatingMultiColor(stripNum, 3);
}

void startFade(uint8_t stripNum) {
	stripState[stripNum].fading = true;
	unsigned int ledFadeSteps = LED_FADE_STEPS
	;
	long colorDist;
	uint32_t rLedColor, rLedColorOld;
	multiColorNextColorTime[stripNum] += (stripState[stripNum].fadeTimeInterval
			* ledFadeSteps);
	switch (stripState[stripNum].ledFadeMode) {
	case 0:
		ledFadeStepTime[stripNum] = 0;
		ledFadeStepIndex[stripNum] = 0;
		for (int x = (stripNumPixels[stripNum] - 1); x >= 0; x--) {
			rLedColor = ledColor[stripNum][x];
			ledColorOld[stripNum][x] = rLedColor;
			rLedColorOld = ledColorOld[stripNum][x];
			colorDist = ledColorFadeTo[stripNum][x] - rLedColorOld;
			ledFadeStep[stripNum][x][0] = (double) (colorDist >> 16)
					/ (double) ledFadeSteps;
			ledFadeStep[stripNum][x][1] = (double) ((colorDist >> 8) & 0xFF)
					/ (double) ledFadeSteps;
			ledFadeStep[stripNum][x][2] = (double) (colorDist & 0xFF)
					/ (double) ledFadeSteps;
			if (0 > colorDist) {
				ledFadeStep[stripNum][x][1] *= -1;
				ledFadeStep[stripNum][x][2] *= -1;
			}
#ifdef DEBUG_MODE
//Serial.print(F("startFade for stripNum -- "));
//Serial.print(stripNum);
//Serial.print(F("; ledColorOld[x] = "));
//Serial.print(ledColorOld[stripNum][x]);
//Serial.print(F(" | "));
//Serial.print(F("ledColorFadeTo[x] = "));
//Serial.print(ledColorFadeTo[stripNum][x]);
//Serial.print(F(" | "));
//Serial.print(F("ledFadeStep[x] = "));
//Serial.print(colorDist & 0xFF);
//Serial.print(F(","));
//Serial.print(ledFadeStep[stripNum][x][0]);
//Serial.print(F(","));
//Serial.print(ledFadeStep[stripNum][x][1]);
//Serial.print(F(","));
//Serial.print(ledFadeStep[stripNum][x][2]);
//Serial.print(F("\n"));
#endif
		}
		break;
	case 1:
	default:
		for (int x = (stripNumPixels[stripNum] - 1); x >= 0; x--) {
			rLedColor = ledColor[stripNum][x];
			ledColorOld[stripNum][x] = rLedColor;
			ledColor[stripNum][x] = ledColorFadeTo[stripNum][x];
		}
		break;
	}
}

void doFade(uint8_t stripNum) {
	if (!stripState[stripNum].fading) {
		return;
	}
	uint16_t ledFadeSteps = LED_FADE_STEPS
	;
	uint32_t rLedColor, rLedColorOld, rLedColorFadeTo;
	switch (stripState[stripNum].ledFadeMode) {
	case 0:
		int x, y;
		if ((uint16_t) ledFadeStepIndex[stripNum] >= ledFadeSteps) {
			// finished, set end color
			for (x = (stripNumPixels[stripNum] - 1); x >= 0; x--) {
				rLedColorFadeTo = ledColorFadeTo[stripNum][x];
				ledColor[stripNum][x] = rLedColorFadeTo;
			}
			stripState[stripNum].fading = false;
		}
		if (ledFadeStepTime[stripNum] - millis()
				> stripState[stripNum].fadeTimeInterval) {
			uint32_t newColor;
			for (x = (stripNumPixels[stripNum] - 1); x >= 0; x--) {
				newColor = ledColorOld[stripNum][x];
				for (y = 2; y >= 0; y--) {
					if (0 > ledFadeStep[stripNum][x][y]) {
						newColor -=
								(long) ((long) ((double) ledFadeStepIndex[stripNum]
										* -ledFadeStep[stripNum][x][y])
										<< (y * 8));
					} else {
						newColor +=
								(long) ((unsigned long) ((double) ledFadeStepIndex[stripNum]
										* ledFadeStep[stripNum][x][y])
										<< (y * 8));
					}
				}
				ledColor[stripNum][x] = newColor;
			}
			ledFadeStepIndex[stripNum]++;
			ledFadeStepTime[stripNum] = millis()
					+ stripState[stripNum].fadeTimeInterval;
			/*
			 #ifdef DEBUG_MODE
			 //Serial.print(F("ledFadeStepIndex = "));
			 //Serial.print(ledFadeStepIndex);
			 //Serial.print(F(" | "));
			 //Serial.print(F("ledFadeStep[0][0] = "));
			 //Serial.print(ledFadeStep[0][0]);
			 //Serial.print(F(" | "));
			 //Serial.print(F("ledColorOld[0] = "));
			 //Serial.print(ledColorOld[0]);
			 //Serial.print(F(" | "));
			 //Serial.print(F("ledColor[0] = "));
			 //Serial.print(ledColor[0]);
			 //Serial.print(F("\n"));
			 #endif
			 */
		}
		break;
	case 1:
	default:
		colorWipe(stripNum,
				ledFadeSteps * stripState[stripNum].fadeTimeInterval);
		stripState[stripNum].fading = false;
		break;
	}
}

/**
 * Throws random colors down the strip array.
 */
void randomCandy(uint8_t stripNum) {
	if (multiColorNextColorTime[stripNum] - millis()
			> stripState[stripNum].multiColorHoldTime) {
		//get new RGB color
		unsigned long new_color = rand()  & 0xFFFFFF;
		// move old color down chain
		for (int x = (stripNumPixels[stripNum] - 1); x > 0; x--) {
			ledColor[stripNum][x] = ledColor[stripNum][x - 1];
		}
		// set new led color
		ledColor[stripNum][0] = new_color;
		multiColorNextColorTime[stripNum] = millis()
				+ stripState[stripNum].multiColorHoldTime;
	}
}

void rainbow(uint8_t stripNum, uint16_t wait) {
	if (multiColorNextColorTime[stripNum] - millis() > wait) {
		int i;

		if (256 < rainbowStepIndex[stripNum]) {
			// 3 cycles of all 256 colors in the wheel
			rainbowStepIndex[stripNum] = 0;
		}
		for (i = 0; i < stripNumPixels[stripNum]; i++) {
			ledColor[stripNum][i] = wheel(
					(i + rainbowStepIndex[stripNum]) % 255);
		}
		rainbowStepIndex[stripNum]++;
		multiColorNextColorTime[stripNum] = millis() + wait;
	}
}

// Slightly different, this one makes the rainbow wheel equally distributed
// along the chain
void rainbowCycle(uint8_t stripNum, uint16_t wait) {
	if (multiColorNextColorTime[stripNum] - millis() > wait) {
		int i;
		if (rainbowStepIndex[stripNum] > 256 * 5) {
			rainbowStepIndex[stripNum] = 0;
		}
		for (i = 0; i < stripNumPixels[stripNum]; i++) {
			// tricky math! we use each pixel as a fraction of the full 96-color wheel
			// (thats the i / strip.numPixels() part)
			// Then add in j which makes the colors go around per pixel
			// the % 96 is to make the wheel cycle around
			ledColor[stripNum][i] = wheel(
					((i * 256 / stripNumPixels[stripNum])
							+ rainbowStepIndex[stripNum]) % 256);
		}
		rainbowStepIndex[stripNum]++;
		multiColorNextColorTime[stripNum] = millis() + wait;
	}
}

// fill the dots one after the other with set colors
// good for testing purposes
void colorWipe(uint8_t stripNum, uint8_t wait) {
	if (stripType[stripNum] != STRIP_TYPE_WS2801
			&& stripType[stripNum] != STRIP_TYPE_WS2811
			&& stripType[stripNum] != STRIP_TYPE_WS2812) {
		return;
	}
	int i;
	for (i = 0; i < stripNumPixels[stripNum]; i++) {
		addressableStrips[stripNum]->setPixelColor(i, ledColor[stripNum][i]);
		addressableStrips[stripNum]->show();
		delay(wait);
	}
}

void renderPixels(uint8_t stripNum) {
	float brightness = stripState[stripNum].ledStripBrightness;
	uint32_t ledColorFadeToChannels[stripNumColorsPerPixel[stripNum]];
	uint32_t brightnessCorrectedColor = 0;
	switch (stripType[stripNum]) {
	case STRIP_TYPE_PWM:
		for (int i = 0; i < stripNumPixels[stripNum]; i++) {
			for (int j = 0; j < stripNumColorsPerPixel[stripNum]; j++) {
				ledColorFadeToChannels[j] = ((float) ((ledColor[stripNum][i]
						>> (8 * j)) & 0xFF) * brightness);
				analogWrite(pwmStripPins[stripNum][j],
						ledColorFadeToChannels[j]);
			}
		}
		break;
	case STRIP_TYPE_WS2801:
	case STRIP_TYPE_WS2811:
	case STRIP_TYPE_WS2812:
	default:
		for (int i = 0; i < stripNumPixels[stripNum]; i++) {
			for (int j = 0; j < stripNumColorsPerPixel[stripNum]; j++) {
				ledColorFadeToChannels[j] = ((float) ((ledColor[stripNum][i]
						>> (8 * j)) & 0xFF) * brightness);
				brightnessCorrectedColor = brightnessCorrectedColor
						| ((uint32_t) ledColorFadeToChannels[j] << (8 * j)); // use rgbColor(...)?
			}
			addressableStrips[stripNum]->setPixelColor(i,
					brightnessCorrectedColor);
		}
		addressableStrips[stripNum]->show();
		break;
	}
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t rgbColor(uint8_t r, uint8_t g, uint8_t b) {
	uint32_t c;
	c = r;
	c <<= 8;
	c |= g;
	c <<= 8;
	c |= b;
	return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t wheel(uint8_t wheelPos) {
	if (wheelPos < 85) {
		return rgbColor(wheelPos * 3, 255 - wheelPos * 3, 0);
	} else if (wheelPos < 170) {
		wheelPos -= 85;
		return rgbColor(255 - wheelPos * 3, 0, wheelPos * 3);
	} else {
		wheelPos -= 170;
		return rgbColor(0, wheelPos * 3, 255 - wheelPos * 3);
	}
}
