#include <string>
#include <cstdlib>
#include "lib/spark-flashee-eeprom/flashee-eeprom.h"
#include "lib/fastled/firmware/FastLED.h"
#include "ledwax_photon.h"
#include "ledwax_photon_util.h"
#include "ledwax_photon_constants.h"

FASTLED_USING_NAMESPACE
using namespace ledwax;

LEDWaxPhoton::LEDWaxPhoton(uint8_t numPixels, uint8_t *stripType, uint8_t *stripNumPixels,
        uint8_t *stripNumColorsPerPixel, uint8_t ***pwmStripPins) {
    // FIXME compile time check
    if (sizeof stripType != sizeof stripNumPixels || sizeof stripType != sizeof stripNumColorsPerPixel
            || sizeof stripNumPixels != sizeof stripNumColorsPerPixel) {
        // invalid params - noop
    }
    this->ledwaxUtil = ledwaxutil::LEDWaxPhotonUtil();
    // init eeprom
    this->flash = Flashee::Devices::createDefaultStore();
    this->numStrips = sizeof stripType / 2;    // wtf - getting 4, despite uint8_t = 1
    this->stripType = &stripType[0];
    this->stripNumPixels = &stripNumPixels[0];
    this->stripNumColorsPerPixel = &stripNumColorsPerPixel[0];
    this->stripState = new led_strip_disp_state[numStrips];
    this->ledModeColorJSONArr = new char[620];
    for (int i = 0; i < numStrips; i++) {
        this->totalNumAddressablePixels += this->stripNumPixels[i];
        if (this->stripNumPixels[i] > this->maxNumPixels) {
            this->maxNumPixels = this->stripNumPixels[i];
        }
    }
    this->stripPins = *pwmStripPins;
    // FIXME replace malloc with new() per c++
    this->multiColorNextColorTime = (uint32_t *) malloc(
            sizeof(uint32_t) * numStrips);
    this->ledColor = (uint32_t **) malloc(
            sizeof(uint32_t *) * numStrips);
    this->ledColorOld = (uint32_t **) malloc(
            sizeof(uint32_t *) * numStrips);
    this->ledColorFadeTo = (uint32_t **) malloc(
            sizeof(uint32_t *) * numStrips);
    this->ledFadeStepTime = (uint32_t *) malloc(
            sizeof(uint32_t) * numStrips);
    this->ledFadeStepIndex = (int16_t *) malloc(
            sizeof(int16_t) * numStrips);
    this->ledFadeStep = (double ***) malloc(
            sizeof(double **) * numStrips);
    this->rainbowStepIndex = (uint8_t *) malloc(
            sizeof(uint8_t) * numStrips);
    this->addressableStrips = (CRGB **) malloc(
            sizeof(CRGB *) * numStrips);
    for (int i = 0; i < numStrips; i++) {
        this->ledColor[i] = (uint32_t *) malloc(
                sizeof(uint32_t) * this->maxNumPixels);
        this->ledColorOld[i] = (uint32_t *) malloc(
                sizeof(uint32_t) * this->maxNumPixels);
        this->ledColorFadeTo[i] = (uint32_t *) malloc(
                sizeof(uint32_t) * this->maxNumPixels);
        this->ledFadeStep[i] = (double **) malloc(
                sizeof(double *) * this->maxNumPixels);
        *this->ledFadeStep[i] = (double *) malloc(
                sizeof(double) * 3);
        this->addressableStrips[i] = new CRGB[this->totalNumAddressablePixels];
//        this->addressableStrips[i] = (CRGB *) malloc(
//                sizeof(CRGB) * this->totalNumAddressablePixels);
        if (ledwaxUtil.isAddressableStrip(
                this->stripType[i])) {
            switch (this->stripType[i]) {
                case STRIP_TYPE_WS2801:
                    FastLED.addLeds<WS2811, A5>(
                            addressableStrips[i], (int) this->stripNumPixels[i], 0);
                    break;
                case STRIP_TYPE_WS2811:
                    FastLED.addLeds<WS2811, A5>(
                            addressableStrips[i], (int) this->stripNumPixels[i], 0);
                    break;
                case STRIP_TYPE_WS2812:
                default:
                    FastLED.addLeds<WS2812, A5>(
                            addressableStrips[i], (int) this->stripNumPixels[i], 0);
                    break;
            }
        } else {
            hasPWMStrip = true;
            addressableStrips[i] = NULL;
        }
    }
    if (hasPWMStrip) {
        pwmDriver = Adafruit_PWMServoDriver(
                0x70);
    }
#if _LWAX_PHOTON_DEBUG_MODE
    this->resetAllStripsToDefault();    // DEV DEBUG -- reset everything on powerup
#endif
}

LEDWaxPhoton::~LEDWaxPhoton() {
    for (int i = 0; i < numStrips; i++) {
        free(this->ledColor[i]);
        free(this->ledColorOld[i]);
        free(this->ledColorFadeTo[i]);
//        free(this->addressableStrips[i]);
        delete this->addressableStrips[i];
        free(this->ledFadeStep[i][0]);
        free(this->ledFadeStep[i][1]);
        free(this->ledFadeStep[i][2]);
        free(this->ledFadeStep[i]);
    }
//    free(this->stripState);
    delete this->stripState;
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

int16_t LEDWaxPhoton::getNumStrips() {
    int ret = numStrips;
    return ret;
}

void LEDWaxPhoton::begin() {
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.begin(19200);
#endif
    remoteControlStripIndex = 0;
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
        }
    }
    if (hasPWMStrip) {
        pwmDriver.begin();
        pwmDriver.setPWMFreq(
                400.0);
    }
}

void LEDWaxPhoton::renderStrips() {
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
    stripState[stripNum].ledModeColorIndex = INITIAL_MULTI_COLOR_ALT_STATE
    ;
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

/**
 * Erase all EEPROM.  Reset states to default, then save the default state in EEPROM.
 */
void LEDWaxPhoton::resetAllStripsToDefault() {
    this->flash->eraseAll();
    for (int i = 0; i < numStrips; i++) {
        this->defaultStripState(
                i);
        saveStripState(
                &stripState[i]);
    }
}

/**
 * Read strip state from stored values in EEPROM.
 */
void LEDWaxPhoton::readStripState(led_strip_disp_state* ret) {
    uint16_t offset = eepromAddyStripState + sizeof(int); // store int at addy 0
    uint8_t bData;
    ret->fading = false;
    flash->read(
            bData, offset++);
    ret->dispMode = (uint8_t) bData;
    flash->read(
            bData, offset++);
    ret->ledFadeMode = (uint8_t) bData;
    uint32_t color;
    for (int i = 0; i < 3; i++) {
        color = 0;
        for (int j = 3; j >= 0; j--) {
            flash->read(
                    bData, offset++);
            color += (bData << (8 * j));
        }
        ret->ledModeColor[i] = color;
    }
    uint32_t holdTime;
    holdTime = 0;
    for (int j = 3; j >= 0; j--) {
        flash->read(
                bData, offset++);
        holdTime += (bData << (8 * j));
    }
    ret->multiColorHoldTime = holdTime;
    uint16_t storedLedStripBrightness;
    storedLedStripBrightness = 0;
    for (int j = 1; j >= 0; j--) {
        flash->read(
                bData, offset++);
        storedLedStripBrightness += (bData << (8 * j));
    }
    ret->ledStripBrightness = (float) storedLedStripBrightness / (float) 1024;
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.print(F("multiColorHoldTime = "));
    Serial.print((ret->multiColorHoldTime));
    Serial.print(F("\nread eeprom done\n"));
#endif
}

/**
 * Save strip state in EEPROM.
 */
void LEDWaxPhoton::saveStripState(led_strip_disp_state* lsds) {
    led_strip_disp_state *storedState = new led_strip_disp_state();
    readStripState(
            storedState);
    uint16_t offset = eepromAddyStripState + sizeof(int); // store int at addy 0
    if (storedState->dispMode != lsds->dispMode) {
        // store byte
        flash->write(
                (uint8_t) lsds->dispMode, offset++);
    }
    offset++;
    if (storedState->ledFadeMode != lsds->ledFadeMode) {
        // store byte
        flash->write(
                (uint8_t) lsds->ledFadeMode, offset++);
    }
    offset++;
    for (int i = 0; i < 3; i++) {
        if (storedState->ledModeColor[i] != lsds->ledModeColor[i]) {
            // store long
            for (int j = 3; j >= 0; j--) {
                flash->write(
                        ((uint8_t) ((lsds->ledModeColor[i] >> (8 * j)) & 0xFF)), offset++);
            }
        } else {
            offset += 4;
        }
    }
    if (storedState->multiColorHoldTime != lsds->multiColorHoldTime) {
        // store long
        for (int j = 3; j >= 0; j--) {
            flash->write(
                    ((uint8_t) ((lsds->multiColorHoldTime >> (8 * j)) & 0xFF)), offset++);
        }
    } else {
        offset += 4;
    }
    if (storedState->ledStripBrightness != lsds->ledStripBrightness) {
        // store int
        uint16_t lsb = lsds->ledStripBrightness * 1024;
        for (int j = 1; j >= 0; j--) {
            flash->write(
                    ((uint8_t) ((lsb >> (8 * j)) & 0xFF)), offset++);
        }
    } else {
        offset += 2;
    }
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.print(F("write eeprom done\n"));
#endif
}

/**
 * @return stripState[stripNum]->ledModeColor as a JSON array
 */
const char * LEDWaxPhoton::buildLedModeColorJSONArr(int stripNum) {
    string ret = "";
    char convBuf[16] = { 32 };
    ret += "[";
    for (int i = 0; i < MAX_NUM_MODE_COLORS; i++) {
        ltoa((long) stripState[stripNum].ledModeColor[i], convBuf, 16);
        ret += (string) convBuf;
        if (i < MAX_NUM_MODE_COLORS - 1) {
            ret += ",";
        }
    }
    ret += "]";
    strncpy(ledModeColorJSONArr, ret.c_str(), 620);
    return ret.c_str();
}

/**
 * Build JSON verson of class state.
 * FIXME: not usable due to photon 620 char limit
 * TODO: use LEDWaxPhoton as param and move to utils; then finish testing when usable
 */
const char * LEDWaxPhoton::buildStripStateJSON() {
    string ret = "";
    char convBuf[5] = { 32, 32, 32, 32, 32 };
    ret += "{ ledStripDisplayState:[";
    for (int i = 0; i < numStrips; i++) {
        ret += "{";
        itoa(stripType[i], convBuf, 10);
        ret += "type:'" + (string) convBuf + "'";
        ret += ",";
        itoa(stripNumPixels[i], convBuf, 10);
        ret += "pixNum:'" + (string) convBuf + "'";
        ret += ",";
        itoa(stripState[i].ledFadeMode, convBuf, 10);
        ret += "fadeMode:'" + (string) convBuf + "'";
        ret += ",";
        itoa(stripState[i].ledStripBrightness, convBuf, 10);
        ret += "bright:'" + (string) convBuf + "'";
        ret += ",";
        ret += "modeColor:[";
        for (int j = 0; j < stripNumColorsPerPixel[i]; j++) {
            itoa(stripState[i].ledModeColor[j], convBuf, 16);
            ret += (string) convBuf;
            if (j < (stripNumColorsPerPixel[i] - 1)) {
                ret += ",";
            }
        }
        ret += "],";
        itoa(stripState[i].ledModeColorIndex, convBuf, 10);
        ret += "mcAltState:'" + (string) convBuf + "'";
        ret += ",";
        ltoa(stripState[i].multiColorHoldTime, convBuf, 16);
        ret += "mcHoldTime:'" + (string) convBuf + "'";
        ret += ",";
        ltoa(stripState[i].fadeTimeInterval, convBuf, 16);
        ret += "fadeTime:'" + (string) convBuf;
        ret += "'";
        ret += "}";
        if (i < numStrips - 1) {
            ret += ",";
        }
    }
    ret += "] }";
    return ret.c_str();
}

/**
 * Set default colors on the given strip for a given display mode.
 */
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

/**
 * Set the current strip being controlled.
 */
int16_t LEDWaxPhoton::setRemoteControlStripIndex(string command) {
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
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.print(F("remoteControlStripIndex = "));
    Serial.print(remoteControlStripIndex);
#endif
    return 0;
}

int16_t LEDWaxPhoton::setModeLEDColor(string command) {
// expect colorindex,value from client apps
    char b1[2];
    char b2[38];
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
    uint32_t color = atol(
            (char *) b2);
    color = (color << 16) >> 16;  // clear high bits
    stripState[remoteControlStripIndex].ledModeColor[colorIndex] = color;
    saveStripState(
            &stripState[remoteControlStripIndex]);
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.print(F("colorIndex = "));
    Serial.print(colorIndex);
    Serial.print(F(","));
    Serial.print(F("color = "));
    Serial.print(color);
    Serial.print(F("\n"));
    Serial.println(data);
    Serial.print(F("\n"));
#endif
    return 0;
}

int16_t LEDWaxPhoton::setDispMode(string command) {
    int bData;
    bData = atoi(
            command.c_str());
    stripState[remoteControlStripIndex].dispMode = bData;
    setDispModeColors(
            remoteControlStripIndex, stripState[remoteControlStripIndex].dispMode);
    stripState[remoteControlStripIndex].ledModeColorIndex = INITIAL_MULTI_COLOR_ALT_STATE
    ;
    stripState[remoteControlStripIndex].fading = false;
    saveStripState(
            &stripState[remoteControlStripIndex]);
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.print(F("dispMode = "));
    Serial.print((int) bData);
    Serial.print(F(", "));
    Serial.print(stripState[remoteControlStripIndex].dispMode);
    Serial.print(F("\n"));
#endif
    return 0;
}

int16_t LEDWaxPhoton::setBright(string command) {
    int bData;
    bData = atoi(
            command.c_str());
    if (255 < bData) {
        bData = 255;
    }
    stripState[remoteControlStripIndex].ledStripBrightness = (float) bData / (float) 255.0;
    saveStripState(
            &stripState[remoteControlStripIndex]);
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.print(F("ledStripBrightness = "));
    Serial.print(bData);
    Serial.print(F(", "));
    Serial.print((float) stripState[remoteControlStripIndex].ledStripBrightness);
    Serial.print(F("\n"));
#endif
    return 0;
}

int16_t LEDWaxPhoton::setLedFadeTimeInterval(string command) {
    uint32_t bData;
    bData = atol(
            command.c_str());
    stripState[remoteControlStripIndex].fadeTimeInterval = bData;
    saveStripState(
            &stripState[remoteControlStripIndex]);
    return 0;
}

int16_t LEDWaxPhoton::setMultiColorHoldTime(string command) {
    uint32_t bData;
    bData = atol(
            command.c_str());
    stripState[remoteControlStripIndex].multiColorHoldTime = bData;
    saveStripState(
            &stripState[remoteControlStripIndex]);
    return 0;
}

int16_t LEDWaxPhoton::setLedFadeMode(string command) {
    uint16_t bData;
    bData = atoi(
            command.c_str());
    if (11 < stripState[remoteControlStripIndex].dispMode) {
        // only for solid colors
        return 1;
    }
    if (1 < bData) {
        bData = 0;
    }
    stripState[remoteControlStripIndex].ledFadeMode = (uint8_t) bData;
    saveStripState(
            &stripState[remoteControlStripIndex]);
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.print(F("ledFadeMode = "));
    Serial.print(stripState[remoteControlStripIndex].ledFadeMode);
    Serial.print(F("\n"));
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
            ledColorFadeTo[stripNum][x] = stripState[stripNum].ledModeColor[stripState[stripNum].ledModeColorIndex];
        }
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
        Serial.print(F("multiColorAltState,ledModeColor[multiColorAltState] = "));
        Serial.print(stripState[stripNum].multiColorAltState);
        Serial.print(F(", "));
        Serial.print(stripState[stripNum].ledModeColor[stripState[stripNum].multiColorAltState]);
        Serial.print(F(", multiColorNextTime,currentMillis = "));
        Serial.print(multiColorNextColorTime[stripNum]);
        Serial.print(F(", "));
        Serial.print(millis());
        Serial.print(F("\n"));
#endif
        stripState[stripNum].ledModeColorIndex = (stripState[stripNum].ledModeColorIndex + 1) % numMultiColors;
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
    uint16_t ledFadeSteps = LED_FADE_STEPS
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
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
                Serial.print(F("startFade for stripNum -- "));
                Serial.print(stripNum);
                Serial.print(F("; ledColorOld[x] = "));
                Serial.print(ledColorOld[stripNum][x]);
                Serial.print(F(" | "));
                Serial.print(F("ledColorFadeTo[x] = "));
                Serial.print(ledColorFadeTo[stripNum][x]);
                Serial.print(F(" | "));
                Serial.print(F("ledFadeStep[x] = "));
                Serial.print(colorDist & 0xFF);
                Serial.print(F(","));
                Serial.print(ledFadeStep[stripNum][x][0]);
                Serial.print(F(","));
                Serial.print(ledFadeStep[stripNum][x][1]);
                Serial.print(F(","));
                Serial.print(ledFadeStep[stripNum][x][2]);
                Serial.print(F("\n"));
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

/**
 * Fade strip colors for time-sliced rendering.
 */
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
                            newColor += (long) ((uint32_t) ((double) ledFadeStepIndex[stripNum]
                                    * ledFadeStep[stripNum][x][y]) << (y * 8));
                        }
                    }
                    ledColor[stripNum][x] = newColor;
                }
                ledFadeStepIndex[stripNum]++;
                ledFadeStepTime[stripNum] = millis() + stripState[stripNum].fadeTimeInterval;
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
                Serial.print(F("ledFadeStepIndex = "));
                Serial.print(ledFadeStepIndex);
                Serial.print(F(" | "));
                Serial.print(F("ledFadeStep[0][0] = "));
                Serial.print(ledFadeStep[0][0]);
                Serial.print(F(" | "));
                Serial.print(F("ledColorOld[0] = "));
                Serial.print(ledColorOld[0]);
                Serial.print(F(" | "));
                Serial.print(F("ledColor[0] = "));
                Serial.print(ledColor[0]);
                Serial.print(F("\n"));
#endif
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
 * Throws random colors down the strip.
 */
void LEDWaxPhoton::randomCandy(uint8_t stripNum) {
    if (multiColorNextColorTime[stripNum] - millis() > stripState[stripNum].multiColorHoldTime) {
        //get new RGB color
        uint32_t new_color = rand() & 0xFFFFFF;
        // move old color down chain
        for (int x = (stripNumPixels[stripNum] - 1); x > 0; x--) {
            ledColor[stripNum][x] = ledColor[stripNum][x - 1];
        }
        // set new led color
        ledColor[stripNum][0] = new_color;
        multiColorNextColorTime[stripNum] = millis() + stripState[stripNum].multiColorHoldTime;
    }
}

/**
 * Cycles the strip through a rainbow wheel of colors.
 */
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

/**
 * Slightly different, this one makes the rainbow wheel equally distributed
 * along the strip.
 */
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

/**
 * Render pixels one after the other with set colors.
 * BLOCKING!  Not time-sliced!
 */
void LEDWaxPhoton::colorWipe(uint8_t stripNum, uint8_t wait) {
    if (stripType[stripNum] != STRIP_TYPE_WS2801 && stripType[stripNum] != STRIP_TYPE_WS2811
            && stripType[stripNum] != STRIP_TYPE_WS2812) {
        return;
    }
    float brightness = stripState[stripNum].ledStripBrightness;
    int brightScale = (int) (brightness * 255.0);
    uint32_t ledColorFadeToChannels[stripNumColorsPerPixel[stripNum]] = { 0 };
    uint32_t brightnessCorrectedColor = 0;
    for (int i = 0; i < stripNumPixels[stripNum]; i++) {
        brightnessCorrectedColor = 0;
        for (int j = 0; j < stripNumColorsPerPixel[stripNum]; j++) {
            ledColorFadeToChannels[j] = ((float) ((ledColor[stripNum][i] >> (8 * j)) & 0xFF) * brightness);
            brightnessCorrectedColor = brightnessCorrectedColor | ((uint32_t) ledColorFadeToChannels[j] << (8 * j)); // use rgbColor(...)?
        }
        addressableStrips[stripNum][i].setColorCode(
                brightnessCorrectedColor);
        FastLED.show();
//        FastLED.showColor(
//                CRGB(ledColor[stripNum][i]));
        delay(wait);
    }
}

/**
 * Render pixels for all strips.  Time-slice compatible.
 */
void LEDWaxPhoton::renderPixels(uint8_t stripNum) {
    float brightness = stripState[stripNum].ledStripBrightness;
    int brightScale = (int) (brightness * 255.0);
    uint32_t ledColorFadeToChannels[stripNumColorsPerPixel[stripNum]] = { 0 };
    uint32_t brightnessCorrectedColor = 0;
    switch (stripType[stripNum]) {
        case STRIP_TYPE_PWM:
            for (int i = 0; i < stripNumPixels[stripNum]; i++) {
                for (int j = 0; j < stripNumColorsPerPixel[stripNum]; j++) {
                    ledColorFadeToChannels[j] = ((float) ((ledColor[stripNum][i] >> (8 * j)) & 0xFF) * brightness);
                    pwmDriver.setPin(
                            stripPins[stripNum][j], ledColorFadeToChannels[j], false);
                }
            }
            break;
        case STRIP_TYPE_WS2801:
        case STRIP_TYPE_WS2811:
        case STRIP_TYPE_WS2812:
        default:
            for (int i = 0; i < stripNumPixels[stripNum]; i++) {
                brightnessCorrectedColor = 0;
                for (int j = 0; j < stripNumColorsPerPixel[stripNum]; j++) {
                    ledColorFadeToChannels[j] = ((float) ((ledColor[stripNum][i] >> (8 * j)) & 0xFF) * brightness);
                    brightnessCorrectedColor = brightnessCorrectedColor
                            | ((uint32_t) ledColorFadeToChannels[j] << (8 * j));
                }
                addressableStrips[stripNum][i].setColorCode(
                        brightnessCorrectedColor);
                FastLED.show();
//                FastLED.showColor(
//                        CRGB(ledColor[stripNum][i]), brightScale);
            }
            break;
    }
}
