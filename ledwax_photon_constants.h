#ifndef LEDWAX_CONSTANTS
#define LEDWAX_CONSTANTS

#ifndef NULL
#define NULL   ((void *) 0)
#endif

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

#define STRIP_TYPE_WS2801 1
#define STRIP_TYPE_WS2811 2
#define STRIP_TYPE_WS2812 3
#define STRIP_TYPE_I2C_PWM  10

#define WIRE_NUM_SPI_1_WIRE 1
#define WIRE_NUM_SPI_2_WIRE 2
#define WIRE_NUM_SPI_3_WIRE 3
#define WIRE_NUM_SPI_4_WIRE 4

#define MAX_NUM_MODE_COLORS 3   // set up to three colors per mode

#define LED_FADE_STEPS 16;
#define LED_FADE_STEP_DELAY_MS 10;  // microsecs between fade steps

#define FADE_MODE_NATIVE_TRANSITION 0
#define FADE_MODE_COLOR_WIPE 1

#define NUM_PIXELS_PER_LED_PWM_WHITE_STRIP  1  // single color PWM
#define NUM_PIXELS_PER_LED_PWM_RGB_STRIP  3  // RGB PWM

#define NUM_LEDS_SPARKFUN_WS2801_1METER 32

#define DEFAULT_MULTI_COLOR_HOLD_TIME 5000;  // time to hold colors when showing multiple colors
#define DEFAULT_DISP_MODE 1;
#define DEFAULT_LED_FADE_MODE FADE_MODE_COLOR_WIPE;
#define INITIAL_MULTI_COLOR_ALT_STATE 0;
#define DEFAULT_LED_STRIP_BRIGHTNESS 1.0;

#endif
