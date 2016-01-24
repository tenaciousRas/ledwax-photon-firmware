#ifndef LEDWAX_SPRITES_H
#define LEDWAX_SPRITES_H
#include "lib/ledsprites/LEDSprites.h"

#define LEDWAX_NUM_FASTLED_SPRITES 3

// cylon
const uint8_t SHAPE_DATA_CYLON[4] = { B8_2BIT(
        32211223), };
const uint8_t SHAPE_MASK_CYLON[4] = { B8_2BIT(
        12233221), };

const CRGB SPRITE_COL_TABLE_SHAPE_CYLON[3] = { CRGB(
        255, 0, 0), CRGB(
        163, 0, 0), CRGB(
        82, 0, 0) };
// dot
const uint8_t SHAPE_DATA_DOT[4] = { B8_2BIT(
        00010000), };
const uint8_t SHAPE_MASK_DOT[4] = { B8_2BIT(
        00030000), };

const CRGB SPRITE_COL_TABLE_SHAPE_DOT[3] = { CRGB(
        255, 0, 0), CRGB(
        0, 0, 0), CRGB(
        0, 0, 0) };

#define SQUARE_WIDTH    6
#define SQUARE_HEIGHT   6
const uint8_t SHAPE_DATA_SQAURE[SQUARE_WIDTH * 4] = { B8_2BIT(
        11111110), B8_2BIT(
        12222210), B8_2BIT(
        12333210), B8_2BIT(
        12333210), B8_2BIT(
        12222210), B8_2BIT(
        11111110) };
const uint8_t SHAPE_MASK_SQUARE[SQUARE_WIDTH * 4] = { B8_2BIT(
        11111110), B8_2BIT(
        11111110), B8_2BIT(
        11111110), B8_2BIT(
        11111110), B8_2BIT(
        11111110), B8_2BIT(
        11111110) };
#endif
