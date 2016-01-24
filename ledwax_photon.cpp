#include <string>
#include <cstdlib>
#include "lib/spark-flashee-eeprom/flashee-eeprom.h"
#include "lib/fastled/firmware/FastLED.h"
#include "ledwax_photon.h"
#include "ledwax_photon_util.h"
#include "ledwax_photon_constants.h"

FASTLED_USING_NAMESPACE
using namespace ledwax;

LEDWaxPhoton::LEDWaxPhoton(uint8_t numStrips, ledwaxconfig::LEDWaxConfig *stripConfigs) {
    // FIXME compile time check
    this->ledwaxUtil = ledwaxutil::LEDWaxPhotonUtil();
    // init eeprom
    this->flash = Flashee::Devices::createDefaultStore();
    this->numStrips = numStrips;
//    this->stripType = &stripType[0];
//    this->stripNumPixels = &stripNumPixels[0];
//    this->stripNumColorsPerPixel = &stripNumColorsPerPixel[0];
    this->stripConfigs = stripConfigs;
    this->stripState = new led_strip_disp_state[numStrips];
    this->ledModeColorJSONArr = new char[620];
    this->multiColorNextColorTime = new uint32_t[numStrips];
    this->ledColor = new uint32_t*[numStrips];
    this->ledColorOld = new uint32_t*[numStrips];
    this->ledColorFadeTo = new uint32_t*[numStrips];
    this->ledFadeStepTime = new uint32_t[numStrips];
    this->ledFadeStepIndex = new int16_t[numStrips];
    this->ledFadeStep = new double*[numStrips];
    this->rainbowStepIndex = new uint16_t[numStrips];
    this->addressableStripPixels = new CRGB*[numStrips];
    this->spritedLEDCanvas = new cLEDMatrix*[numStrips];
    this->sprites = new cLEDSprites*[numStrips];
    this->spriteShapes = new cSprite**[numStrips];
    this->spriteShapesColorTables = new CRGB**[numStrips];
    this->fastLEDControllers = new CLEDController*[numStrips];
    this->pwmDriver = new Adafruit_PWMServoDriver*[numStrips];
    uint16_t numFastLEDControllersDefd = 0, prevFastLEDControllerStripIndex = 0, fastLEDControllerLEDOffset = 0;
    for (int i = 0; i < numStrips; i++) {
        this->ledColor[i] = new uint32_t[this->stripConfigs[i].getNumPixels()];
        this->ledColorOld[i] = new uint32_t[this->stripConfigs[i].getNumPixels()];
        this->ledColorFadeTo[i] = new uint32_t[this->stripConfigs[i].getNumPixels()];
        this->ledFadeStep[i] = new double[this->stripConfigs[i].getNumPixels() * 3];
        this->addressableStripPixels[i] = new CRGB[this->stripConfigs[i].getNumPixels()];
        if (ledwaxUtil.isAddressableStrip(
                this->stripConfigs[i].getStripType())) {
            // smelly way to distinguish SPI ports
            if (this->stripConfigs[prevFastLEDControllerStripIndex].getSpiPins()[0]
                    != this->stripConfigs[i].getSpiPins()[0]) {
                fastLEDControllerLEDOffset = 0;
            }
            switch (this->stripConfigs[i].getStripType()) {
                case STRIP_TYPE_WS2801:
                    if (fastLEDControllerLEDOffset < 1) {
                        // TODO runtime pin configuration requires refactor of FastLED c'tor
                        // C++ templates lock us into compile-time configuration
                        if (numFastLEDControllersDefd == 0) {
                            fastLEDControllers[i] = &(FastLED.addLeds<WS2811_400, A5>(
                                    addressableStripPixels[i], (int) this->stripConfigs[i].getNumPixels()));
                        } else if (numFastLEDControllersDefd == 1) {
                            fastLEDControllers[i] = &(FastLED.addLeds<WS2811_400, A6>(
                                    addressableStripPixels[i], (int) this->stripConfigs[i].getNumPixels()));
                        }
                        prevFastLEDControllerStripIndex = i;
                        numFastLEDControllersDefd++;
                    } else {
                        FastLED.addLeds(
                                fastLEDControllers[prevFastLEDControllerStripIndex], addressableStripPixels[i],
                                fastLEDControllerLEDOffset, (int) this->stripConfigs[i].getNumPixels());
                    }
                    break;
                case STRIP_TYPE_WS2811:
                    if (fastLEDControllerLEDOffset < 1) {
                        if (numFastLEDControllersDefd == 0) {
                            fastLEDControllers[i] = &(FastLED.addLeds<WS2811, A5>(
                                    addressableStripPixels[i], (int) this->stripConfigs[i].getNumPixels()));
                        } else if (numFastLEDControllersDefd == 1) {
                            fastLEDControllers[i] = &(FastLED.addLeds<WS2811, A6>(
                                    addressableStripPixels[i], (int) this->stripConfigs[i].getNumPixels()));
                        }
                        prevFastLEDControllerStripIndex = i;
                        numFastLEDControllersDefd++;
                    } else {
                        FastLED.addLeds(
                                fastLEDControllers[prevFastLEDControllerStripIndex], addressableStripPixels[i],
                                fastLEDControllerLEDOffset, (int) this->stripConfigs[i].getNumPixels());
                    }
                    break;
                case STRIP_TYPE_WS2812:
                default:
                    if (fastLEDControllerLEDOffset < 1) {
                        if (numFastLEDControllersDefd == 0) {
                            fastLEDControllers[i] = &(FastLED.addLeds<WS2812, A5>(
                                    addressableStripPixels[i], (int) this->stripConfigs[i].getNumPixels()));
                        } else if (numFastLEDControllersDefd == 1) {
                            fastLEDControllers[i] = &(FastLED.addLeds<WS2812, A6>(
                                    addressableStripPixels[i], (int) this->stripConfigs[i].getNumPixels()));
                        }
                        prevFastLEDControllerStripIndex = i;
                        numFastLEDControllersDefd++;
                    } else {
                        FastLED.addLeds(
                                fastLEDControllers[prevFastLEDControllerStripIndex], addressableStripPixels[i],
                                fastLEDControllerLEDOffset, (int) this->stripConfigs[i].getNumPixels());
                    }
                    break;
            }
            fastLEDControllerLEDOffset += (uint16_t) this->stripConfigs[i].getNumPixels();
            this->spritedLEDCanvas[i] = new cLEDMatrix(
                    HORIZONTAL_MATRIX, this->stripConfigs[i].getMatrixWidth(), this->stripConfigs[i].getMatrixHeight());
            this->spritedLEDCanvas[i]->SetLEDArray(
                    addressableStripPixels[i]);
            this->sprites[i] = new cLEDSprites(
                    this->spritedLEDCanvas[i]);
            this->spriteShapes[i] = new cSprite*[LEDWAX_NUM_FASTLED_SPRITES];    // hardcoded
            this->spriteShapesColorTables[i] = new CRGB*[LEDWAX_NUM_FASTLED_SPRITES];    // hardcoded
            this->pwmDriver[i] = NULL;
        } else {
            hasPWMStrip = true;
            fastLEDControllers[i] = NULL;
            addressableStripPixels[i] = NULL;
            this->spritedLEDCanvas[i] = NULL;
            this->spriteShapes[i] = NULL;
            pwmDriver[i] = new Adafruit_PWMServoDriver(
                    0x70);
        }
    }
    if (hasPWMStrip) {
    }
}

LEDWaxPhoton::~LEDWaxPhoton() {
    for (int i = 0; i < numStrips; i++) {
        delete this->ledColor[i];
        delete this->ledColorOld[i];
        delete this->ledColorFadeTo[i];
        delete this->addressableStripPixels[i];
        delete this->ledFadeStep[i];
    }
    delete this->stripState;
    delete this->multiColorNextColorTime;
    delete this->ledColor;
    delete this->ledColorOld;
    delete this->ledColorFadeTo;
    delete this->ledFadeStepTime;
    delete this->ledFadeStepIndex;
    delete this->ledFadeStep;
    delete this->rainbowStepIndex;
    delete this->addressableStripPixels;
}

int16_t LEDWaxPhoton::getNumStrips() {
    int ret = numStrips;
    return ret;
}

void LEDWaxPhoton::begin() {
#ifdef _LWAX_PHOTON_DEBUG_MODE
    this->resetAllStripsToDefault();    // DEV DEBUG -- reset everything on powerup
#endif
#ifdef _LWAX_PHOTON_SERIAL_DEBUG_MODE
    Serial.begin(19200);
#endif
    remoteControlStripIndex = 0;
    for (int i = 0; i < numStrips; i++) {
#ifndef _LWAX_PHOTON_DEBUG_MODE
        defaultStripState(
                i);
        // read eeprom
        readStripState(
                &stripState[i]);
#endif
        // setup LEDs
        setDispModeColors(
                i, stripState[i].dispMode);
        // setup Sprites
        initSprites(
                i);
        // setup fading
        multiColorNextColorTime[i] = getTimeToFadeNow(
                i);
        startFade(
                (uint8_t) i);
        if (pwmDriver[i] != NULL) {
            pwmDriver[i]->begin();
            pwmDriver[i]->setPWMFreq(
                    400.0);
        }
    }
}

void LEDWaxPhoton::renderStrips() {
    for (int i = 0; i < numStrips; i++) {
        switch (stripState[i].dispMode) {
            case 30:
                // hack so we don't fade
                // sprites operate directly on CRGB structure
                break;
            default:
                if (stripState[i].fading) {
                    doFade(i);
                    renderPixels(
                            i);
                    continue;
                }
                break;
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
                alternatingTwoColors(
                        i);
                break;
            case 16:
                alternatingThreeColors(
                        i);
                break;
            case 20:
                rainbow(i, stripState[i].multiColorHoldTime);
                break;
            case 21:
                rainbowCycle(
                        i, stripState[i].multiColorHoldTime);
                break;
            case 22:
                randomCandy(
                        i);
                break;
            case 30:
            case 31:
            case 32:
                animatedSprite(
                        i);
                break;
            default:
                break;
        }
        switch (stripState[i].dispMode) {
            case 30:
            case 31:
            case 32:
                // hack so we don't fade
                break;
            default:
                if (multiColorNextColorTime[i] - millis() > stripState[i].multiColorHoldTime) {
                    multiColorNextColorTime[i] = millis() + stripState[i].multiColorHoldTime;
                    startFade(
                            (uint8_t) i);
                }
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
    stripState[stripNum].setLedStripBrightness(
            (uint16_t) 1024);
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
    uint16_t storedLedStripBrightness = 0;
    for (int j = 1; j >= 0; j--) {
        flash->read(
                bData, offset++);
        storedLedStripBrightness += (bData << (8 * j));
    }
    ret->setLedStripBrightness(
            storedLedStripBrightness);
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
        uint16_t lsb = lsds->ledStripBrightnessScale;
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
    delete storedState;
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
        itoa(this->stripConfigs[i].getStripType(), convBuf, 10);
        ret += "type:'" + (string) convBuf + "'";
        ret += ",";
        itoa(this->stripConfigs[i].getNumPixels(), convBuf, 10);
        ret += "pixNum:'" + (string) convBuf + "'";
        ret += ",";
        itoa(stripState[i].ledFadeMode, convBuf, 10);
        ret += "fadeMode:'" + (string) convBuf + "'";
        ret += ",";
        itoa(stripState[i].ledStripBrightnessScale, convBuf, 10);
        ret += "bright:'" + (string) convBuf + "'";
        ret += ",";
        ret += "modeColor:[";
        for (int j = 0; j < this->stripConfigs[i].getNumColorsPerPixel(); j++) {
            itoa(stripState[i].ledModeColor[j], convBuf, 16);
            ret += (string) convBuf;
            if (j < (this->stripConfigs[i].getNumColorsPerPixel() - 1)) {
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
            stripState[stripNum].ledModeColor[0] = random(
                    0xFFFFFF);
            stripState[stripNum].ledModeColor[1] = random(
                    0xFFFFFF);
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
            stripState[stripNum].ledModeColor[2] = OFF;
            break;
        case 14:
            break;
        case 15:
            stripState[stripNum].ledModeColor[0] = random(
                    0xFFFFFF);
            stripState[stripNum].ledModeColor[1] = random(
                    0xFFFFFF);
            break;
        case 16:
            stripState[stripNum].ledModeColor[0] = random(
                    0xFFFFFF);
            stripState[stripNum].ledModeColor[1] = random(
                    0xFFFFFF);
            stripState[stripNum].ledModeColor[2] = random(
                    0xFFFFFF);
            break;
        case 20:
            rainbowStepIndex[stripNum] = 0;
            break;
        case 21:
            rainbowStepIndex[stripNum] = 0;
            break;
        case 22:
            break;
        case 30:
        case 31:
        case 32:
            this->sprites[stripNum]->RemoveAllSprites();
            this->sprites[stripNum]->AddSprite(
                    this->spriteShapes[stripNum][mode - 30]);
            setSpriteColors(
                    stripNum);
            allLEDsOFF(
                    stripNum);
            break;
        default:
            // noop
            break;
    }
}

void LEDWaxPhoton::initSprites(uint8_t stripNum) {
    if (!ledwaxUtil.isAddressableStrip(
            stripConfigs[stripNum].getStripType())) {
        return;
    }
    initCylonSprite(
            stripNum);
    initDotSprite(
            stripNum);
    initSquareSprite(
            stripNum);
}

void LEDWaxPhoton::initCylonSprite(uint8_t stripNum) {
    this->spriteShapesColorTables[stripNum][0] = new CRGB[4];
    this->spriteShapes[stripNum][0] = new cSprite(
            8, 1, SHAPE_DATA_CYLON, 1, _2BIT, spriteShapesColorTables[stripNum][0], SHAPE_MASK_CYLON);
    this->spriteShapes[stripNum][0]->SetPositionFrameMotionOptions(
            0/*X*/, 0/*Y*/, 0/*Frame*/, 0/*FrameRate*/, +1/*XChange*/, 1/*XRate*/, +0/*YChange*/, 1/*YRate*/,
            SPRITE_DETECT_EDGE | SPRITE_X_KEEPIN | SPRITE_Y_KEEPIN);
}

void LEDWaxPhoton::initDotSprite(uint8_t stripNum) {
    this->spriteShapesColorTables[stripNum][1] = new CRGB[4];
    this->spriteShapes[stripNum][1] = new cSprite(
            8, 1, SHAPE_DATA_DOT, 1, _2BIT, spriteShapesColorTables[stripNum][1], SHAPE_MASK_DOT);
    this->spriteShapes[stripNum][1]->SetPositionFrameMotionOptions(
            0/*X*/, 0/*Y*/, 0/*Frame*/, 0/*FrameRate*/, +1/*XChange*/, 1/*XRate*/, +0/*YChange*/, 1/*YRate*/,
            SPRITE_DETECT_EDGE | SPRITE_X_KEEPIN | SPRITE_Y_KEEPIN);
}

void LEDWaxPhoton::initSquareSprite(uint8_t stripNum) {
    this->spriteShapesColorTables[stripNum][2] = new CRGB[4];
    this->spriteShapes[stripNum][2] = new cSprite(
            6, 6, SHAPE_DATA_SQAURE, 1, _2BIT, spriteShapesColorTables[stripNum][2], SHAPE_MASK_SQUARE);
    this->spriteShapes[stripNum][2]->SetPositionFrameMotionOptions(
            0/*X*/, 0/*Y*/, 0/*Frame*/, 0/*FrameRate*/, +1/*XChange*/, 1/*XRate*/, +1/*YChange*/, 1/*YRate*/,
            SPRITE_DETECT_EDGE | SPRITE_X_KEEPIN | SPRITE_Y_KEEPIN);
}

void LEDWaxPhoton::setSpriteColors(int stripNum) {
    if (!ledwaxUtil.isAddressableStrip(
            stripConfigs[stripNum].getStripType())) {
        return;
    }
    // loop over [cylon, dot, square] sprites
    for (int i = 0; i < 3; i++) {
        spriteShapesColorTables[stripNum][i][0].setColorCode(
                stripState[stripNum].ledModeColor[0]);
        spriteShapesColorTables[stripNum][i][1].setColorCode(
                stripState[stripNum].ledModeColor[0]);
        spriteShapesColorTables[stripNum][i][2].setColorCode(
                stripState[stripNum].ledModeColor[0]);
        spriteShapesColorTables[stripNum][i][3].setColorCode(
                0);
        spriteShapesColorTables[stripNum][i][0].fadeToBlackBy(
                255 - (uint8_t) (stripState[stripNum].ledStripBrightnessScale >> 2));
        spriteShapesColorTables[stripNum][i][1].fadeToBlackBy(
                255 - (uint8_t) (stripState[stripNum].ledStripBrightnessScale >> 3));
        spriteShapesColorTables[stripNum][i][2].fadeToBlackBy(
                255 - (uint8_t) (stripState[stripNum].ledStripBrightnessScale >> 5));
    }
}

/**
 * Set the current strip being controlled.
 */
int16_t LEDWaxPhoton::setRemoteControlStripIndex(string command) {
    int bData;
    bData = atoi(
            command.c_str());
    bData = max(
            bData, 0);
    bData = min(
            bData, numStrips - 1);
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
    color = (color << 8) >> 8;  // clear high bits
    colorIndex = max(
            colorIndex, 0);
    colorIndex = min(
            colorIndex, MAX_NUM_MODE_COLORS - 1);
    stripState[remoteControlStripIndex].ledModeColor[colorIndex] = color;
    saveStripState(
            &stripState[remoteControlStripIndex]);
    setSpriteColors(
            remoteControlStripIndex);
    multiColorNextColorTime[remoteControlStripIndex] = getTimeToFadeNow(
            remoteControlStripIndex);
    stripState[remoteControlStripIndex].fading = false;
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
    bData = max(
            bData, 0);
    bData = min(
            bData, 32);
    stripState[remoteControlStripIndex].dispMode = bData;
    setDispModeColors(
            remoteControlStripIndex, stripState[remoteControlStripIndex].dispMode);
    stripState[remoteControlStripIndex].ledModeColorIndex = INITIAL_MULTI_COLOR_ALT_STATE
    ;
    saveStripState(
            &stripState[remoteControlStripIndex]);
    multiColorNextColorTime[remoteControlStripIndex] = getTimeToFadeNow(
            remoteControlStripIndex);
    stripState[remoteControlStripIndex].fading = false;
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
    stripState[remoteControlStripIndex].setLedStripBrightness(
            (uint16_t) bData);
    saveStripState(
            &stripState[remoteControlStripIndex]);
    setSpriteColors(
            remoteControlStripIndex);
    if (!stripState[remoteControlStripIndex].fading) {
        renderPixels(
                remoteControlStripIndex);
    }
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
    bData = max(
            bData, (uint32_t) 1);
    bData = min(
            bData, (uint32_t) 65535);
    stripState[remoteControlStripIndex].fadeTimeInterval = bData;
    saveStripState(
            &stripState[remoteControlStripIndex]);
    return 0;
}

int16_t LEDWaxPhoton::setMultiColorHoldTime(string command) {
    uint32_t bData;
    bData = atol(
            command.c_str());
    bData = max(
            bData, (uint32_t) 0);
    bData = min(
            bData, (uint32_t) 65535);
    stripState[remoteControlStripIndex].multiColorHoldTime = bData;
    saveStripState(
            &stripState[remoteControlStripIndex]);
    multiColorNextColorTime[remoteControlStripIndex] = getTimeToFadeNow(
            remoteControlStripIndex);
    stripState[remoteControlStripIndex].fading = false;
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
    bData = max(
            bData, (uint16_t) 0);
    bData = min(
            bData, (uint16_t) 1);
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

void LEDWaxPhoton::allLEDsOFF(uint8_t stripNum) {
    for (int i = 0; i < this->stripConfigs[stripNum].getNumPixels(); i++) {
        ledColor[stripNum][i] = OFF;
        ledColorFadeTo[stripNum][i] = OFF;
        switch (stripState[stripNum].dispMode) {
            case 30:
            case 31:
            case 32:
                addressableStripPixels[stripNum][i].setColorCode(
                        0);
                break;
            default:
                break;
        }
    }
}

void LEDWaxPhoton::allLEDsWhite(uint8_t stripNum) {
    for (int i = 0; i < this->stripConfigs[stripNum].getNumPixels(); i++) {
        ledColor[stripNum][i] = WHITE;
        ledColorFadeTo[stripNum][i] = OFF;
    }
}

void LEDWaxPhoton::solidMultiColor(uint8_t stripNum, int numMultiColors) {
    if (multiColorNextColorTime[stripNum] - millis() > stripState[stripNum].multiColorHoldTime) {
        for (int x = this->stripConfigs[stripNum].getNumPixels() - 1; x >= 0; x--) {
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
    }
}

void LEDWaxPhoton::alternatingMultiColor(uint8_t stripNum, int numMultiColors) {
    for (int x = (this->stripConfigs[stripNum].getNumPixels() - 1); x >= 0; x -= numMultiColors) {
        for (int y = 0; y < numMultiColors; y++) {
            ledColorFadeTo[stripNum][x - y] = stripState[stripNum].ledModeColor[y];
        }
    }
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

void LEDWaxPhoton::alternatingThreeColors(uint8_t stripNum) {
    alternatingMultiColor(
            stripNum, 3);
}

uint32_t LEDWaxPhoton::getTimeToFadeNow(uint8_t stripNum) {
    return (uint32_t) millis() - (stripState[stripNum].multiColorHoldTime << 2)
            - (stripState[stripNum].fadeTimeInterval * numLEDFadeSteps);
}

void LEDWaxPhoton::startFade(uint8_t stripNum) {
    stripState[stripNum].fading = true;
    long colorDist;
    uint32_t rLedColor, rLedColorOld;
    ledFadeStepTime[stripNum] = (uint32_t) millis() + stripState[stripNum].fadeTimeInterval;
    ledFadeStepIndex[stripNum] = 0;
    switch (stripState[stripNum].ledFadeMode) {
        case FADE_MODE_NATIVE_TRANSITION:
            multiColorNextColorTime[stripNum] += (stripState[stripNum].fadeTimeInterval * numLEDFadeSteps);
            for (int x = (this->stripConfigs[stripNum].getNumPixels() - 1); x >= 0; x--) {
                rLedColor = ledColor[stripNum][x];
                ledColorOld[stripNum][x] = rLedColor;
                rLedColorOld = ledColorOld[stripNum][x];
                colorDist = (long) ledColorFadeTo[stripNum][x] - (long) rLedColorOld;
                ledFadeStep[stripNum][x * 3] = (double) ((double) (colorDist >> 16) / (double) numLEDFadeSteps);
                ledFadeStep[stripNum][x * 3 + 1] = (double) ((double) (colorDist >> 16) / (double) numLEDFadeSteps);
                ledFadeStep[stripNum][x * 3 + 2] = (double) ((double) (colorDist >> 16) / (double) numLEDFadeSteps);
                if (0 > colorDist) {
                    ledFadeStep[stripNum][x * 3 + 1] *= -1;
                    ledFadeStep[stripNum][x * 3 + 1] *= -1;
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
        case FADE_MODE_COLOR_WIPE:
        default:
            multiColorNextColorTime[stripNum] += (stripState[stripNum].fadeTimeInterval
                    * this->stripConfigs[stripNum].getNumPixels());
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
    uint32_t rLedColorFadeTo;
    switch (stripState[stripNum].ledFadeMode) {
        case FADE_MODE_NATIVE_TRANSITION:
            int x, y;
            if (ledFadeStepIndex[stripNum] >= numLEDFadeSteps) {
                // finished, set end color
                for (x = (this->stripConfigs[stripNum].getNumPixels() - 1); x >= 0; x--) {
                    rLedColorFadeTo = ledColorFadeTo[stripNum][x];
                    ledColor[stripNum][x] = rLedColorFadeTo;
                }
                stripState[stripNum].fading = false;
            }
            if (ledFadeStepTime[stripNum] - millis() > stripState[stripNum].fadeTimeInterval) {
                uint32_t newColor;
                for (x = (this->stripConfigs[stripNum].getNumPixels() - 1); x >= 0; x--) {
                    newColor = ledColorOld[stripNum][x];
                    for (y = 2; y >= 0; y--) {
                        if (0 > ledFadeStep[stripNum][x * 3 + y]) {
                            newColor -= (long) ((long) ((double) ledFadeStepIndex[stripNum]
                                    * -ledFadeStep[stripNum][x * 3 + y]) << (y * 8));
                        } else {
                            newColor += (uint32_t) ((double) ledFadeStepIndex[stripNum]
                                    * ledFadeStep[stripNum][x * 3 + y]) << (y * 8);
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
        case FADE_MODE_COLOR_WIPE:
        default:
            if (ledFadeStepIndex[stripNum] >= this->stripConfigs[stripNum].getNumPixels() - 1) {
                // finished
                stripState[stripNum].fading = false;
                break;
            }
            if (ledFadeStepTime[stripNum] - millis() > stripState[stripNum].fadeTimeInterval) {
                // use ledFadeStepIndex as index of pixel being wiped
                ledColor[stripNum][ledFadeStepIndex[stripNum]] = ledColorFadeTo[stripNum][ledFadeStepIndex[stripNum]];
                ledFadeStepIndex[stripNum]++;
                ledFadeStepTime[stripNum] = millis() + stripState[stripNum].fadeTimeInterval;
            }
            break;
    }
}

/**
 * Throws random colors down the strip.
 */
void LEDWaxPhoton::randomCandy(uint8_t stripNum) {
    if (multiColorNextColorTime[stripNum] - millis() > (uint32_t) stripState[stripNum].multiColorHoldTime) {
        // move old color down chain
        for (int x = (int16_t) this->stripConfigs[stripNum].getNumPixels() - 1; x > 0; x--) {
            ledColorFadeTo[stripNum][x] = ledColor[stripNum][x - 1];
        }
        // set new led color
        ledColorFadeTo[stripNum][0] = (uint32_t) random(
                0xFFFFFF);
    }
}

/**
 * Render all animated sprite modes.
 */
void LEDWaxPhoton::animatedSprite(uint8_t stripNum) {
    if (ledFadeStepTime[stripNum] - millis() > stripState[stripNum].fadeTimeInterval) {
        ledFadeStepTime[stripNum] = (uint32_t) millis() + stripState[stripNum].fadeTimeInterval;
        renderPixels(
                stripNum);
    }
}

/**
 * Cycles the strip through a rainbow wheel of colors.
 */
void LEDWaxPhoton::rainbow(uint8_t stripNum, uint16_t wait) {
    if (multiColorNextColorTime[stripNum] - millis() > wait) {
        int i = 0;
        if (rainbowStepIndex[stripNum] <= 0) {
            // 3 cycles of all 256 colors in the wheel
            rainbowStepIndex[stripNum] = 256 * 3;
        }
        for (i = 0; i < this->stripConfigs[stripNum].getNumPixels(); i++) {
            ledColorFadeTo[stripNum][i] = ledwaxUtil.wheel(
                    (i + rainbowStepIndex[stripNum]) % 255);
        }
        rainbowStepIndex[stripNum]--;
    }
}

/**
 * Slightly different, this one makes the rainbow wheel equally distributed
 * along the strip.
 */
void LEDWaxPhoton::rainbowCycle(uint8_t stripNum, uint16_t wait) {
    if (multiColorNextColorTime[stripNum] - millis() > wait) {
        int i;
        if (rainbowStepIndex[stripNum] <= 0) {
            rainbowStepIndex[stripNum] = 256 * 5;
        }
        for (i = 0; i < this->stripConfigs[stripNum].getNumPixels(); i++) {
            // Use each pixel as a fraction of the full 256-color wheel
            // (thats the i / strip.getNumPixels()() part).
            // Then add in j which makes the colors go around per pixel
            // the % 256 is to make the wheel cycle around
            ledColorFadeTo[stripNum][i] = ledwaxUtil.wheel(
                    ((i * 256 / this->stripConfigs[stripNum].getNumPixels()) + rainbowStepIndex[stripNum]) % 255);
        }
        rainbowStepIndex[stripNum]--;
    }
}

/**
 * Render pixels for all strips.  Time-slice compatible.
 */
void LEDWaxPhoton::renderPixels(uint8_t stripNum) {
    uint16_t *ledColorFadeToChannels = new uint16_t[this->stripConfigs[stripNum].getNumColorsPerPixel()];
    switch (this->stripConfigs[stripNum].getStripType()) {
        case STRIP_TYPE_I2C_PWM:
            for (int i = 0; i < this->stripConfigs[stripNum].getNumPixels(); i++) {
                for (int j = 0; j < this->stripConfigs[stripNum].getNumColorsPerPixel(); j++) {
                    ledColorFadeToChannels[j] = (uint16_t) ((float) ((ledColor[stripNum][i] >> (8 * j)) & 0xFF)
                            / (stripState[stripNum].ledStripBrightnessScale / 4));
                    pwmDriver[stripNum]->setPin(
                            stripConfigs[stripNum].getI2cPwmPins()[j], ledColorFadeToChannels[j], true);
                }
            }
            break;
        case STRIP_TYPE_WS2801:
        case STRIP_TYPE_WS2811:
        case STRIP_TYPE_WS2812:
        default:
            switch (stripState[stripNum].dispMode) {
                case 30:
                case 31:
                case 32:
                    fastLEDControllers[stripNum]->clearLedData();
                    sprites[stripNum]->UpdateSprites();
                    sprites[stripNum]->RenderSprites();
                    break;
                default:
                    for (int i = 0; i < this->stripConfigs[stripNum].getNumPixels(); i++) {
                        addressableStripPixels[stripNum][i].setColorCode(
                                ledColor[stripNum][i]);
                    }
                    break;
            }
            fastLEDControllers[stripNum]->showLeds(
                    this->stripState[stripNum].getLedStripBrightness8Bit());
            break;
    }
    delete ledColorFadeToChannels;
}
