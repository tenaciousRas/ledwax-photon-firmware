#ifndef LEDWAX_CONFIG_H
#define LEDWAX_CONFIG_H
#include "ledwax_photon_constants.h"

namespace ledwaxconfig {
    class LEDWaxConfig {
    private:

    public:
        uint8_t stripType;
        uint8_t numPixels;
        uint8_t numColorsPerPixel;
        // formatting break
        bool matrix;
        uint8_t matrixHeight;
        uint8_t matrixWidth;
        uint8_t *spiPins = NULL;
        uint8_t *i2cPins = NULL;
        uint8_t *i2cPWMPins = NULL;
        uint8_t *nativePWMPins = NULL;

        LEDWaxConfig() {
            this->stripType = 0;
            this->numPixels = 0;
            this->numColorsPerPixel = 0;
            this->matrix = false;
            this->matrixHeight = 1;
            this->matrixWidth = 0;
            this->spiPins = NULL;
            this->i2cPins = NULL;
            this->i2cPWMPins = NULL;
            this->nativePWMPins = NULL;
        }

        LEDWaxConfig(uint8_t stripType, uint8_t numPixels, uint8_t numColorsPerPixel, uint8_t *spiPins = NULL,
                uint8_t *i2cPins = NULL, uint8_t *i2cPWMPins = NULL, uint8_t *nativePWMPins = NULL) {
            this->stripType = stripType;
            this->numPixels = numPixels;
            this->numColorsPerPixel = numColorsPerPixel;
            this->matrix = false;
            this->matrixHeight = 1;
            this->matrixWidth = numPixels;
            this->spiPins = spiPins;
            this->i2cPins = i2cPins;
            this->i2cPWMPins = i2cPWMPins;
            this->nativePWMPins = nativePWMPins;
        }

        LEDWaxConfig(uint8_t stripType, uint8_t numPixels, uint8_t numColorsPerPixel, bool matrix = false,
                uint8_t matrixHeight = 1, uint8_t matrixWidth = 0, uint8_t *spiPins = NULL, uint8_t *i2cPins = NULL,
                uint8_t *i2cPWMPins = NULL, uint8_t *nativePWMPins = NULL) {
            this->stripType = stripType;
            this->numPixels = numPixels;
            this->numColorsPerPixel = numColorsPerPixel;
            this->matrix = matrix;
            this->matrixHeight = matrixHeight;
            this->matrixWidth = matrixWidth;
            this->spiPins = spiPins;
            this->i2cPins = i2cPins;
            this->i2cPWMPins = i2cPWMPins;
            this->nativePWMPins = nativePWMPins;
        }

        ~LEDWaxConfig() {
        }

    };
}
#endif
