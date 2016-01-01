#ifndef LEDWAX_H
#define LEDWAX_H
#include "lib/spark-flashee-eeprom/flashee-eeprom.h"
#include "lib/fastled/firmware/FastLED.h"
#include "lib/Adafruit_PWMServoDriver/Adafruit_PWMServoDriver.h"
#include "ledwax_photon_util.h"
#include "ledwax_photon_constants.h"

FASTLED_USING_NAMESPACE
using namespace std;

#define _LWAX_PHOTON_VERSION 5  // it's probably more like v.100
#define _LWAX_PHOTON_DEBUG_MODE 1

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

#define LED_FADE_STEPS 16;
#define LED_FADE_STEP_DELAY_MS 50;  // microsecs between fade steps

namespace ledwax {
    class LEDWaxPhoton {

    public:

        LEDWaxPhoton(uint8_t[], uint8_t[], uint8_t[], uint8_t[][NUM_PIXELS_PER_LED_PWM_RGB_STRIP]);
        ~LEDWaxPhoton();

        typedef struct {
            uint8_t dispMode;
            bool fading;
            uint8_t ledFadeMode; // color fade mode, 0 for entire strip, 1 for swipe pixels
            int16_t multiColorAltState;  // state of alternating colors
            uint32_t ledModeColor[3];
            uint32_t multiColorHoldTime;
            uint32_t fadeTimeInterval;
            float ledStripBrightness;
        } led_strip_disp_state;

        led_strip_disp_state *stripState;
        int16_t numStrips = 0;
        int16_t maxNumPixels = 0;
        int16_t totalNumAddressablePixels = 0;
        char stripStateJSON[620];
        uint8_t remoteControlStripIndex = 0;

        // METHOD DECLARATIONS
        void begin(), renderAll(), defaultStripState(uint8_t), readStripState(led_strip_disp_state*), putStripState(
                led_strip_disp_state*), setDispModeColors(uint8_t, int), refreshLEDs(uint8_t), turnOffLEDs(uint8_t),
                white(uint8_t), solidMultiColor(uint8_t, int), alternatingMultiColor(uint8_t, int), solidOneColor(
                        uint8_t), solidTwoColors(uint8_t), solidThreeColors(uint8_t), alternatingTwoColors(uint8_t),
                alternatingTwoRandomColors(uint8_t), alternatingThreeColors(uint8_t), startFade(uint8_t), doFade(
                        uint8_t), randomCandy(uint8_t), rainbow(uint8_t, uint16_t), rainbowCycle(uint8_t, uint16_t),
                colorWipe(uint8_t, uint8_t), renderPixels(uint8_t);
        string
        buildStripStateJSON();
        char*
        getStripStateJSON();
        int16_t getNumStrips();
        int16_t setLEDParams(string), setRemoteControlStripIndex(string), setLEDStripColor(string), setDispMode(string),
                setBright(string), setLedFadeTimeInterval(string), setMultiColorHoldTime(string), setLedFadeMode(
                        string);
        uint32_t rgbColor(uint8_t, uint8_t, uint8_t), wheel(uint8_t);

    private:
        ledwaxutil::LEDWaxPhotonUtil ledwaxUtil;

        uint8_t *stripType, *stripNumPixels, // 1 for PWM strip
                *stripNumColorsPerPixel;
        CRGB **addressableStrips;
        // Adafruit_NeoPixel* addressableStrips[];
        // Initialize strip variables.  Interesting C implementation.  Define two arrays, one for
        // addressable strips, one for PWM.  Effectively define position of strips by populating specific members
        // of each array.
        // FIXME improve implementation
        uint8_t **pwmStripPins;

        uint32_t *multiColorNextColorTime;
        uint32_t **ledColor;
//	{ {
//	RED,
//	OFF }, { TWYELLOW, TWBLUE }
// };
        //  {RED, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF},
        //  {TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE, TWYELLOW, TWBLUE}
        uint32_t **ledColorOld;
//	{ { RED,
//	OFF }, { TWYELLOW, TWBLUE } };  // color before fade
        uint32_t **ledColorFadeTo;
//	{ {
//	RED,
//	OFF }, { TWYELLOW, TWBLUE } };    // fade-to color
        uint32_t *ledFadeStepTime;  // time to next fade step
        int16_t *ledFadeStepIndex; // color distance divided by LED_FADE_STEPS
        double ***ledFadeStep; // 3 for each RGB component
        uint8_t *rainbowStepIndex;

        Flashee::FlashDevice* flash;
        int16_t eepromAddyStripState = 4;  // eeprom addy to store strip state

        bool hasPWMStrip = false;
        Adafruit_PWMServoDriver pwmDriver;
    };
}
#endif
