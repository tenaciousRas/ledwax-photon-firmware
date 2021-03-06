#ifndef LEDWAX_H
#define LEDWAX_H
#include "application.h"
#include "lib/spark-flashee-eeprom/flashee-eeprom.h"
#include "lib/fastled/firmware/FastLED.h"
#include "lib/Adafruit_PWMServoDriver/Adafruit_PWMServoDriver.h"
#include "lib/ledmatrix/LEDMatrix.h"
#include "lib/ledsprites/LEDSprites.h"
#include "ledwax_photon_sprites.h"
#include "ledwax_photon_util.h"
#include "ledwax_photon_constants.h"
#include "ledwax_photon_config.h"

FASTLED_USING_NAMESPACE
using namespace std;

#define _LWAX_PHOTON_VERSION 8  // it's probably more like v.100
#define _LWAX_PHOTON_DEBUG_MODE 1   // comment out to disable
// #define _LWAX_PHOTON_SERIAL_DEBUG_MODE 1   // comment to disable

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
 * 16: three random alternating colors
 * 20: rainbow
 * 21: rainbow cycle
 * 22: random candy
 * 30: cylon
 */
namespace ledwax {
    class LEDWaxPhoton {

    public:

        LEDWaxPhoton(uint8_t, ledwaxconfig::LEDWaxConfig *stripConfigs);
        ~LEDWaxPhoton();

        typedef struct {
            uint8_t dispMode;bool fading;
            uint8_t ledFadeMode; // color fade mode, 0 for entire strip, 1 for swipe pixels
            uint8_t ledModeColorIndex;  // state of alternating colors
            uint32_t ledModeColor[MAX_NUM_MODE_COLORS];
            uint32_t multiColorHoldTime;
            uint32_t fadeTimeInterval;
            float ledStripBrightness;
            uint16_t ledStripBrightnessScale;
            void setLedStripBrightness(float bright) {
                bright = max(
                        bright, 0.0);
                bright = min(
                        bright, 1.0);
                ledStripBrightness = bright;
                ledStripBrightnessScale = (uint16_t) ledStripBrightness * 1023;
            }
            ;
            void setLedStripBrightness(uint16_t bright) {
                bright = max(
                        bright, 0);
                bright = min(
                        bright, 1023);
                ledStripBrightnessScale = bright;
                ledStripBrightness = (float) ledStripBrightnessScale / (float) 1023;
            }
            ;
            uint8_t getLedStripBrightness8Bit() {
                return (uint8_t) (ledStripBrightnessScale / 4);
            }
        } led_strip_disp_state;

        ledwaxconfig::LEDWaxConfig *stripConfigs;
        led_strip_disp_state *stripState;
        char *ledModeColorJSONArr;    // stored for particle var
        int16_t numStrips = 0;
        int16_t totalNumAddressablePixels = 0;
//        uint8_t *stripType, *stripNumPixels, *stripNumColorsPerPixel;
//        uint8_t *stripPins;
        uint8_t remoteControlStripIndex = 0;

        // METHOD DECLARATIONS
        void begin(), renderStrips(), defaultStripState(uint8_t), readStripState(led_strip_disp_state*), saveStripState(
                led_strip_disp_state*), setDispModeColors(uint8_t, int), refreshLEDs(uint8_t), allLEDsOFF(uint8_t),
                allLEDsWhite(uint8_t), solidMultiColor(uint8_t, int), alternatingMultiColor(uint8_t, int),
                solidOneColor(uint8_t), solidTwoColors(uint8_t), solidThreeColors(uint8_t), alternatingTwoColors(
                        uint8_t), alternatingTwoRandomColors(uint8_t), alternatingThreeColors(uint8_t), startFade(
                        uint8_t), doFade(uint8_t), randomCandy(uint8_t), rainbow(uint8_t, uint16_t), rainbowCycle(
                        uint8_t, uint16_t), animatedSprite(uint8_t), colorWipe(uint8_t, uint8_t), renderPixels(uint8_t),
                resetAllStripsToDefault(), setSpriteColors(int), initSprites(uint8_t), initCylonSprite(uint8_t),
                initDotSprite(uint8_t), initSquareSprite(uint8_t);
        const char *buildStripStateJSON(), *buildLedModeColorJSONArr(int);
        int16_t getNumStrips();
        int16_t setRemoteControlStripIndex(string), setModeLEDColor(string), setDispMode(string), setBright(string),
                setLedFadeTimeInterval(string), setMultiColorHoldTime(string), setLedFadeMode(string);
        uint32_t rgbColor(uint8_t, uint8_t, uint8_t), wheel(uint8_t);
        uint32_t getTimeToFadeNow(uint8_t);

    private:
        ledwaxutil::LEDWaxPhotonUtil ledwaxUtil;

        CLEDController** fastLEDControllers;
        CRGB** addressableStripPixels;
        cLEDMatrix** spritedLEDCanvas;
        uint32_t* multiColorNextColorTime;
        // FIXME refactor for CRGB
        uint32_t** ledColor;
        uint32_t** ledColorOld;
        uint32_t** ledColorFadeTo;
        uint32_t* ledFadeStepTime;  // time to next fade step
        int16_t* ledFadeStepIndex;  // fade step
        double** ledFadeStep;   // color distance divided by LED_FADE_STEPS
        uint16_t numLEDFadeSteps = LED_FADE_STEPS
        ;
        uint16_t* rainbowStepIndex;

        Flashee::FlashDevice* flash;
        int16_t eepromAddyStripState = 4;  // eeprom addy to store strip state

        bool hasPWMStrip = false;
        Adafruit_PWMServoDriver** pwmDriver;

        cLEDSprites** sprites;
        cSprite*** spriteShapes;
        CRGB*** spriteShapesColorTables;
    };
}
#endif
