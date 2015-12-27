#include <string>
#include "lib/spark-flashee-eeprom/flashee-eeprom.h"
#include "lib/neopixel/neopixel.h"
#include "ledwax_photon.h"
#include "ledwax_photon_util.h"
#include "ledwax_photon_constants.h"

using namespace ledwax;

LEDWaxPhoton::LEDWaxPhoton(uint8_t stripType[], uint8_t stripNumPixels[], uint8_t stripNumColorsPerPixel[],
        uint8_t pwmStripPins[][NUM_LEDS_PWM_RGB_STRIP]) {
    if (sizeof(stripType) != sizeof(stripNumPixels) || sizeof(stripType) != sizeof(stripNumColorsPerPixel)
            || sizeof(stripNumPixels) != sizeof(stripNumColorsPerPixel)) {
        // invalid params
        return;
    }
    this->ledwaxUtil = ledwaxutil::LEDWaxPhotonUtil();
    // init eeprom
    flash = Flashee::Devices::createDefaultStore();
    this->numStrips = sizeof(stripType);
    *this->stripType = *stripType;
    *this->stripNumPixels = *stripNumPixels;
    *this->stripNumColorsPerPixel = *stripNumColorsPerPixel;
    *this->stripState = *(led_strip_disp_state *) malloc(
            sizeof(led_strip_disp_state) * numStrips);
    for (int i = 0; i < numStrips; i++) {
        if (*this->stripNumPixels > maxNumPixels) {
            this->maxNumPixels = *this->stripNumPixels;
        }
    }
    *this->pwmStripPins[0] = pwmStripPins[0][0];
    *this->multiColorNextColorTime = *(unsigned long *) malloc(
            sizeof(unsigned long) * numStrips);
    **this->ledColor = *(unsigned long *) malloc(
            sizeof(unsigned long) * numStrips * this->maxNumPixels);
    **this->ledColorOld = *(unsigned long *) malloc(
            sizeof(unsigned long) * numStrips * this->maxNumPixels);
    **this->ledColorFadeTo = *(unsigned long *) malloc(
            sizeof(unsigned long) * numStrips * this->maxNumPixels);
    *this->ledFadeStepTime = *(uint16_t *) malloc(
            sizeof(unsigned long) * numStrips);
    *this->ledFadeStepIndex = *(uint16_t *) malloc(
            sizeof(int) * numStrips);
    ***this->ledFadeStep = *(double *) malloc(
            sizeof(unsigned long) * numStrips * this->maxNumPixels * 3);
    *this->rainbowStepIndex = *(uint16_t *) malloc(
            sizeof(int) * numStrips);
    *this->addressableStrips = (Adafruit_NeoPixel *) malloc(
            sizeof(Adafruit_NeoPixel*) * numStrips);
    for (int i = 0; i < numStrips; i++) {
        if (ledwaxUtil.isAddressableStrip(
                this->stripType[i])) {
            addressableStrips[i] = new Adafruit_NeoPixel(
                    (uint8_t) *this->stripNumPixels, 5, WS2811);
        } else {
            hasPWMStrip = true;
            addressableStrips[i] = NULL;
        }
    }
    if (hasPWMStrip) {
        pwmDriver = Adafruit_PWMServoDriver(
                0x0);
    }
}

LEDWaxPhoton::~LEDWaxPhoton() {
    free(this->stripState);
    free(this->multiColorNextColorTime);
    free(this->ledColor);
    free(this->ledColorOld);
    free(this->ledColorFadeTo);
    free(this->ledFadeStepTime);
    free(this->ledFadeStepIndex);
    free(this->ledFadeStep);
    free(this->rainbowStepIndex);
    free(this->addressableStrips);
}

void LEDWaxPhoton::begin() {
    remoteControlStripIndex = 0;
    bool hasPWMStrip = true;   // only setup I2C if PWM strip def'd
    for (int i = 0; i < numStrips; i++) {
        defaultStripState(
                i);
        // read eeprom
        readStripState(
                &stripState[i]);
        // setup LEDs
        setDispModeColors(
                i, stripState[i].dispMode);
        if (ledwaxUtil.isAddressableStrip(
                this->stripType[i])) {
            hasPWMStrip = false;
            addressableStrips[i]->begin();
            addressableStrips[i]->show();
        }
    }
    if (hasPWMStrip) {
        pwmDriver.begin();
        pwmDriver.setPWMFreq(
                400.0);
    }
}

void LEDWaxPhoton::renderAll() {
    for (int i = 0; i < numStrips; i++) {
        if (stripState[i].fading) {
            doFade(i);
            renderPixels(
                    i);
            continue;
        }
        switch (stripState[i].dispMode) {
            case 0:
                solidOneColor(
                        i);
                break;
            case 1:
                solidTwoColors(
                        i);
                break;
            case 2:
                solidTwoColors(
                        i);
                break;
            case 10:
                solidTwoColors(
                        i);
                break;
            case 11:
                solidThreeColors(
                        i);
                break;
            case 12:
                alternatingTwoColors(
                        i);
                break;
            case 13:
                alternatingTwoColors(
                        i);
                break;
            case 14:
                alternatingThreeColors(
                        i);
                break;
            case 15:
                alternatingTwoRandomColors(
                        i);
                break;
            case 20:
                rainbow(i, 1000);
                break;
            case 21:
                rainbowCycle(
                        i, 1000);
                break;
            case 22:
                randomCandy(
                        i);
                break;
            case 30:
                break;
            default:
                break;
        }
    }
}

void LEDWaxPhoton::defaultStripState(uint8_t stripNum) {
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

void LEDWaxPhoton::readStripState(led_strip_disp_state* ret) {
    unsigned int offset = eepromAddyStripState + sizeof(int); // store int at addy 0
    uint8_t bData;
    ret->fading = false;
    ret->multiColorAltState = INITIAL_MULTI_COLOR_ALT_STATE
    ;
    flash->read(
            bData, offset++);
    ret->dispMode = (uint8_t) bData;
    flash->read(
            bData, offset++);
    ret->ledFadeMode = (uint8_t) bData;
    unsigned long color;
    for (int i = 0; i < 3; i++) {
        color = 0;
        for (int j = 3; j >= 0; j--) {
            flash->read(
                    bData, offset++);
            color += (bData << (8 * j));
        }
        ret->ledModeColor[i] = color;
    }
    unsigned long holdTime;
    holdTime = 0;
    for (int j = 3; j >= 0; j--) {
        flash->read(
                bData, offset++);
        holdTime += (bData << (8 * j));
    }
    ret->multiColorHoldTime = holdTime;
    unsigned int storedLedStripBrightness;
    storedLedStripBrightness = 0;
    for (int j = 1; j >= 0; j--) {
        flash->read(
                bData, offset++);
        storedLedStripBrightness += (bData << (8 * j));
    }
    ret->ledStripBrightness = (float) storedLedStripBrightness / (float) 1024;
#ifdef DEBUG_MODE
//  //Serial.print(F("multiColorHoldTime = "));
//  //Serial.print((ret->multiColorHoldTime));
//  //Serial.print(F("\nread eeprom done\n"));
#endif
}

void LEDWaxPhoton::putStripState(led_strip_disp_state* lsds) {
    led_strip_disp_state storedState;
    readStripState(
            &storedState);
    unsigned int offset = eepromAddyStripState + sizeof(int); // store int at addy 0
    if ((unsigned int) storedState.dispMode != (unsigned int) lsds->dispMode) {
        // store byte
        flash->write(
                (uint8_t) lsds->dispMode, offset++);
    }
    offset++;
    if (storedState.ledFadeMode != lsds->ledFadeMode) {
        // store byte
        flash->write(
                (uint8_t) lsds->ledFadeMode, offset++);
    }
    offset++;
    for (int i = 0; i < 3; i++) {
        if (storedState.ledModeColor[i] != lsds->ledModeColor[i]) {
            // store long
            for (int j = 3; j >= 0; j--) {
                flash->write(
                        ((uint8_t) ((lsds->ledModeColor[i] >> (8 * j)) & 0xFF)), offset++);
            }
        } else {
            offset += 4;
        }
    }
    if (storedState.multiColorHoldTime != lsds->multiColorHoldTime) {
        // store long
        for (int j = 3; j >= 0; j--) {
            flash->write(
                    ((uint8_t) ((lsds->multiColorHoldTime >> (8 * j)) & 0xFF)), offset++);
        }
    } else {
        offset += 4;
    }
    if (storedState.ledStripBrightness != lsds->ledStripBrightness) {
        // store int
        unsigned int lsb = lsds->ledStripBrightness * 1024;
        for (int j = 1; j >= 0; j--) {
            flash->write(
                    ((uint8_t) ((lsb >> (8 * j)) & 0xFF)), offset++);
        }
    } else {
        offset += 2;
    }
    // update strip state watchvar
    stripStateJSON = buildStripStateJSON().c_str();
    Particle.publish(
            "ledStripDisplayState", stripStateJSON);
#ifdef DEBUG_MODE
//  //Serial.print(F("write eeprom done\n"));
#endif
}

string LEDWaxPhoton::buildStripStateJSON() {
    string ret = "";
    char convBuf[5] = { 0 };
    ret += "{ ledStripDisplayState:[";
    for (int i = 0; i < numStrips; i++) {
        ret += "{";
        itoa(stripType[i], convBuf, 2);
        ret += "stripType:'" + (string) convBuf + "'";
        ret += ",";
        itoa(stripNumPixels[i], convBuf, 2);
        ret += "stripNumPixels:'" + (string) convBuf + "'";
        ret += ",";
        itoa(stripState[i].ledFadeMode, convBuf, 2);
        ret += "ledFadeMode:'" + (string) convBuf + "'";
        ret += ",";
        itoa(stripState[i].ledStripBrightness, convBuf, 2);
        ret += "ledStripBrightness:'" + (string) convBuf + "'";
        ret += ",";
        ret += "ledModeColor:[";
        for (int j = 0; j < stripNumColorsPerPixel[i]; j++) {
            itoa(stripState[i].ledModeColor[j], convBuf, 2);
            ret += (string) convBuf;
            if (j < (stripNumColorsPerPixel[i] - 1)) {
                ret += ",";
            }
        }
        ret += "],";
        itoa(stripState[i].multiColorAltState, convBuf, 2);
        ret += "multiColorAltState:'" + (string) convBuf + "'";
        ret += ",";
        //ltoa(stripState[i].multiColorHoldTime, convBuf, 4);
        ret += "multiColorHoldTime:'" + (string) convBuf + "'";
        ret += ",";
        //ltoa(stripState[i].fadeTimeInterval, convBuf, 4);
        ret += "fadeTimeInterval:'" + (string) convBuf;
        ret += "'";
        ret += "}";
        if (i < numStrips - 1) {
            ret += ",";
        }
    }
    ret += "] }";
    return ret;
}

void LEDWaxPhoton::setDispModeColors(uint8_t stripNum, int mode) {
    switch (mode) {
        case 0:
            break;
        case 1:
            stripState[stripNum].ledModeColor[0] = TWYELLOW;
            stripState[stripNum].ledModeColor[1] = TWBLUE;
            break;
        case 2:
            stripState[stripNum].ledModeColor[0] = rand() & 0xFFFFFF;
            stripState[stripNum].ledModeColor[1] = rand() & 0xFFFFFF;
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
            stripState[stripNum].ledModeColor[0] = rand() & 0xFFFFFF;
            stripState[stripNum].ledModeColor[1] = rand() & 0xFFFFFF;
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
}

int LEDWaxPhoton::setRemoteControlStripIndex(string command) {
    int bData;
    bData = atoi(
            command.c_str());
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

int LEDWaxPhoton::setLEDStripColor(string command) {
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
    int colorIndex = atoi(
            (char *) b1);
    unsigned long color = atol(
            (char *) b2);
    color = (color << 8) >> 8;
    stripState[remoteControlStripIndex].ledModeColor[colorIndex] = color;
    putStripState(
            &stripState[remoteControlStripIndex]);
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

int LEDWaxPhoton::setDispMode(string command) {
    int bData;
    bData = atoi(
            command.c_str());
    stripState[remoteControlStripIndex].dispMode = bData;
    setDispModeColors(
            remoteControlStripIndex, stripState[remoteControlStripIndex].dispMode);
    stripState[remoteControlStripIndex].multiColorAltState = INITIAL_MULTI_COLOR_ALT_STATE
    ;
    stripState[remoteControlStripIndex].fading = false;
    putStripState(
            &stripState[remoteControlStripIndex]);
#ifdef DEBUG_MODE
//Serial.print(F("dispMode = "));
//Serial.print((int) bData);
//Serial.print(F(", "));
//Serial.print(stripState[remoteControlStripIndex].dispMode);
//Serial.print(F("\n"));
#endif
    return 0;
}

int LEDWaxPhoton::setBright(string command) {
    int bData;
    bData = atoi(
            command.c_str());
    if (255 < bData) {
        bData = 255;
    }
    stripState[remoteControlStripIndex].ledStripBrightness = (float) bData / (float) 255.0;
    putStripState(
            &stripState[remoteControlStripIndex]);
#ifdef DEBUG_MODE
//Serial.print(F("ledStripBrightness = "));
//Serial.print(bData);
//Serial.print(F(", "));
//Serial.print((float) stripState[remoteControlStripIndex].ledStripBrightness);
//Serial.print(F("\n"));
#endif
    return 0;
}

int LEDWaxPhoton::setLedFadeTimeInterval(string command) {
    unsigned long bData;
    bData = atol(
            command.c_str());
    stripState[remoteControlStripIndex].fadeTimeInterval = bData;
    putStripState(
            &stripState[remoteControlStripIndex]);
    return 0;
}

int LEDWaxPhoton::setMultiColorHoldTime(string command) {
    unsigned long bData;
    bData = atol(
            command.c_str());
    stripState[remoteControlStripIndex].multiColorHoldTime = bData;
    putStripState(
            &stripState[remoteControlStripIndex]);
    return 0;
}

int LEDWaxPhoton::setLedFadeMode(string command) {
    unsigned int bData;
    bData = atoi(
            command.c_str());
    if (11 < stripState[remoteControlStripIndex].dispMode) {
        // only for solid colors
        return 1;
    }
    if (1 < bData) {
        bData = 0;
    }
    stripState[remoteControlStripIndex].ledFadeMode = bData;
    putStripState(
            &stripState[remoteControlStripIndex]);
#ifdef DEBUG_MODE
//Serial.print(F("ledFadeMode = "));
//Serial.print(stripState[remoteControlStripIndex].ledFadeMode);
//Serial.print(F("\n"));
#endif
    return 0;
}

void LEDWaxPhoton::refreshLEDs(uint8_t stripNum) {
// TODO implement with realtime fader
}

void LEDWaxPhoton::turnOffLEDs(uint8_t stripNum) {
// TODO re-implement
    for (int i = 0; i < stripNumPixels[stripNum]; i++) {
        ledColor[stripNum][i] = OFF;
    }
}

void LEDWaxPhoton::white(uint8_t stripNum) {
// TODO re-implement
    for (int i = 0; i < stripNumPixels[stripNum]; i++) {
        ledColor[stripNum][i] = WHITE;
    }
}

void LEDWaxPhoton::solidMultiColor(uint8_t stripNum, int numMultiColors) {
    if (multiColorNextColorTime[stripNum] - millis() > stripState[stripNum].multiColorHoldTime) {
        multiColorNextColorTime[stripNum] = millis() + stripState[stripNum].multiColorHoldTime;
        for (int x = stripNumPixels[stripNum] - 1; x >= 0; x--) {
            ledColorFadeTo[stripNum][x] = stripState[stripNum].ledModeColor[stripState[stripNum].multiColorAltState];
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
        stripState[stripNum].multiColorAltState = (stripState[stripNum].multiColorAltState + 1) % numMultiColors;
        startFade(
                stripNum);
    }
}

void LEDWaxPhoton::alternatingMultiColor(uint8_t stripNum, int numMultiColors) {
    for (int x = (stripNumPixels[stripNum] - 1); x >= 0; x -= numMultiColors) {
        for (int y = 0; y < numMultiColors; y++) {
            ledColorFadeTo[stripNum][x - y] = stripState[stripNum].ledModeColor[y];
        }
    }
    startFade(
            stripNum);
}

void LEDWaxPhoton::solidOneColor(uint8_t stripNum) {
    solidMultiColor(
            stripNum, 1);
}

void LEDWaxPhoton::solidTwoColors(uint8_t stripNum) {
    solidMultiColor(
            stripNum, 2);
}

void LEDWaxPhoton::solidThreeColors(uint8_t stripNum) {
    solidMultiColor(
            stripNum, 3);
}

void LEDWaxPhoton::alternatingTwoColors(uint8_t stripNum) {
    alternatingMultiColor(
            stripNum, 2);
}

void LEDWaxPhoton::alternatingTwoRandomColors(uint8_t stripNum) {
    alternatingMultiColor(
            stripNum, 2);
}

void LEDWaxPhoton::alternatingThreeColors(uint8_t stripNum) {
    alternatingMultiColor(
            stripNum, 3);
}

void LEDWaxPhoton::startFade(uint8_t stripNum) {
    stripState[stripNum].fading = true;
    unsigned int ledFadeSteps = LED_FADE_STEPS
    ;
    long colorDist;
    uint32_t rLedColor, rLedColorOld;
    multiColorNextColorTime[stripNum] += (stripState[stripNum].fadeTimeInterval * ledFadeSteps);
    switch (stripState[stripNum].ledFadeMode) {
        case 0:
            ledFadeStepTime[stripNum] = 0;
            ledFadeStepIndex[stripNum] = 0;
            for (int x = (stripNumPixels[stripNum] - 1); x >= 0; x--) {
                rLedColor = ledColor[stripNum][x];
                ledColorOld[stripNum][x] = rLedColor;
                rLedColorOld = ledColorOld[stripNum][x];
                colorDist = ledColorFadeTo[stripNum][x] - rLedColorOld;
                ledFadeStep[stripNum][x][0] = (double) (colorDist >> 16) / (double) ledFadeSteps;
                ledFadeStep[stripNum][x][1] = (double) ((colorDist >> 8) & 0xFF) / (double) ledFadeSteps;
                ledFadeStep[stripNum][x][2] = (double) (colorDist & 0xFF) / (double) ledFadeSteps;
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

void LEDWaxPhoton::doFade(uint8_t stripNum) {
    if (!stripState[stripNum].fading) {
        return;
    }
    uint16_t ledFadeSteps = LED_FADE_STEPS
    ;
    uint32_t rLedColorFadeTo;
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
            if (ledFadeStepTime[stripNum] - millis() > stripState[stripNum].fadeTimeInterval) {
                uint32_t newColor;
                for (x = (stripNumPixels[stripNum] - 1); x >= 0; x--) {
                    newColor = ledColorOld[stripNum][x];
                    for (y = 2; y >= 0; y--) {
                        if (0 > ledFadeStep[stripNum][x][y]) {
                            newColor -= (long) ((long) ((double) ledFadeStepIndex[stripNum]
                                    * -ledFadeStep[stripNum][x][y]) << (y * 8));
                        } else {
                            newColor += (long) ((unsigned long) ((double) ledFadeStepIndex[stripNum]
                                    * ledFadeStep[stripNum][x][y]) << (y * 8));
                        }
                    }
                    ledColor[stripNum][x] = newColor;
                }
                ledFadeStepIndex[stripNum]++;
                ledFadeStepTime[stripNum] = millis() + stripState[stripNum].fadeTimeInterval;
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
            colorWipe(
                    stripNum, ledFadeSteps * stripState[stripNum].fadeTimeInterval);
            stripState[stripNum].fading = false;
            break;
    }
}

/**
 * Throws random colors down the strip array.
 */
void LEDWaxPhoton::randomCandy(uint8_t stripNum) {
    if (multiColorNextColorTime[stripNum] - millis() > stripState[stripNum].multiColorHoldTime) {
        //get new RGB color
        unsigned long new_color = rand() & 0xFFFFFF;
        // move old color down chain
        for (int x = (stripNumPixels[stripNum] - 1); x > 0; x--) {
            ledColor[stripNum][x] = ledColor[stripNum][x - 1];
        }
        // set new led color
        ledColor[stripNum][0] = new_color;
        multiColorNextColorTime[stripNum] = millis() + stripState[stripNum].multiColorHoldTime;
    }
}

void LEDWaxPhoton::rainbow(uint8_t stripNum, uint16_t wait) {
    if (multiColorNextColorTime[stripNum] - millis() > wait) {
        int i;

        if (256 < rainbowStepIndex[stripNum]) {
            // 3 cycles of all 256 colors in the wheel
            rainbowStepIndex[stripNum] = 0;
        }
        for (i = 0; i < stripNumPixels[stripNum]; i++) {
            ledColor[stripNum][i] = ledwaxUtil.wheel(
                    (i + rainbowStepIndex[stripNum]) % 255);
        }
        rainbowStepIndex[stripNum]++;
        multiColorNextColorTime[stripNum] = millis() + wait;
    }
}

// Slightly different, this one makes the rainbow wheel equally distributed
// along the chain
void LEDWaxPhoton::rainbowCycle(uint8_t stripNum, uint16_t wait) {
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
            ledColor[stripNum][i] = ledwaxUtil.wheel(
                    ((i * 256 / stripNumPixels[stripNum]) + rainbowStepIndex[stripNum]) % 256);
        }
        rainbowStepIndex[stripNum]++;
        multiColorNextColorTime[stripNum] = millis() + wait;
    }
}

// fill the dots one after the other with set colors
// good for testing purposes
void LEDWaxPhoton::colorWipe(uint8_t stripNum, uint8_t wait) {
    if (stripType[stripNum] != STRIP_TYPE_WS2801 && stripType[stripNum] != STRIP_TYPE_WS2811
            && stripType[stripNum] != STRIP_TYPE_WS2812) {
        return;
    }
    int i;
    for (i = 0; i < stripNumPixels[stripNum]; i++) {
        addressableStrips[stripNum]->setPixelColor(
                i, ledColor[stripNum][i]);
        addressableStrips[stripNum]->show();
        delay(wait);
    }
}

void LEDWaxPhoton::renderPixels(uint8_t stripNum) {
    float brightness = stripState[stripNum].ledStripBrightness;
    uint32_t ledColorFadeToChannels[stripNumColorsPerPixel[stripNum]];
    uint32_t brightnessCorrectedColor = 0;
    switch (stripType[stripNum]) {
        case STRIP_TYPE_PWM:
            for (int i = 0; i < stripNumPixels[stripNum]; i++) {
                for (int j = 0; j < stripNumColorsPerPixel[stripNum]; j++) {
                    ledColorFadeToChannels[j] = ((float) ((ledColor[stripNum][i] >> (8 * j)) & 0xFF) * brightness);
                    pwmDriver.setPin(
                            pwmStripPins[stripNum][j], ledColorFadeToChannels[j], false);
                }
            }
            break;
        case STRIP_TYPE_WS2801:
        case STRIP_TYPE_WS2811:
        case STRIP_TYPE_WS2812:
        default:
            for (int i = 0; i < stripNumPixels[stripNum]; i++) {
                for (int j = 0; j < stripNumColorsPerPixel[stripNum]; j++) {
                    ledColorFadeToChannels[j] = ((float) ((ledColor[stripNum][i] >> (8 * j)) & 0xFF) * brightness);
                    brightnessCorrectedColor = brightnessCorrectedColor
                            | ((uint32_t) ledColorFadeToChannels[j] << (8 * j)); // use rgbColor(...)?
                }
                addressableStrips[stripNum]->setPixelColor(
                        i, brightnessCorrectedColor);
            }
            addressableStrips[stripNum]->show();
            break;
    }
}
