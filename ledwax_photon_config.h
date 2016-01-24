#ifndef LEDWAX_CONFIG_H
#define LEDWAX_CONFIG_H
#include "ledwax_photon_constants.h"

namespace ledwaxconfig {
    class LEDWaxConfig {
    private:
        // formatting break
        bool matrix;
        uint8_t matrixHeight;
        uint8_t matrixWidth;
        uint8_t stripType;
        uint8_t numPixels;
        uint8_t numColorsPerPixel;
        uint8_t *spiPins = NULL;
        uint16_t i2cAddy = 0;
        uint8_t *i2cPins = NULL;
        uint8_t *i2cPWMPins = NULL;
        uint8_t *nativePWMPins = NULL;

    public:

        LEDWaxConfig() {
            this->stripType = 0;
            this->numPixels = 0;
            this->numColorsPerPixel = 0;
            this->matrix = false;
            this->matrixHeight = 0;
            this->matrixWidth = 0;
            this->spiPins = NULL;
            this->i2cAddy = 0;
            this->i2cPins = NULL;
            this->i2cPWMPins = NULL;
            this->nativePWMPins = NULL;
        }

        LEDWaxConfig(uint8_t stripType, uint8_t numPixels, uint8_t numColorsPerPixel, uint8_t *spiPins = NULL,
                uint16_t i2cAddy = 0, uint8_t *i2cPins = NULL, uint8_t *i2cPWMPins = NULL, uint8_t *nativePWMPins = NULL) {
            this->stripType = stripType;
            this->numPixels = numPixels;
            this->numColorsPerPixel = numColorsPerPixel;
            this->matrix = false;
            initMatrixDimensions(
                    matrix);
            this->matrixHeight = 1;
            this->matrixWidth = numPixels;
            this->spiPins = spiPins;
            this->i2cAddy = i2cAddy;
            this->i2cPins = i2cPins;
            this->i2cPWMPins = i2cPWMPins;
            this->nativePWMPins = nativePWMPins;
        }

        LEDWaxConfig(uint8_t stripType, uint8_t numPixels, uint8_t numColorsPerPixel, bool matrix = false,
                uint8_t matrixHeight = 1, uint8_t *spiPins = NULL, uint16_t i2cAddy = 0, uint8_t *i2cPins = NULL, uint8_t *i2cPWMPins = NULL,
                uint8_t *nativePWMPins = NULL) {
            this->stripType = stripType;
            this->numPixels = numPixels;
            this->numColorsPerPixel = numColorsPerPixel;
            this->matrix = matrix;
            this->matrixHeight = matrixHeight;
            initMatrixDimensions(
                    matrix);
            this->spiPins = spiPins;
            this->i2cPins = i2cPins;
            this->i2cAddy = i2cAddy;
            this->i2cPWMPins = i2cPWMPins;
            this->nativePWMPins = nativePWMPins;
        }

        ~LEDWaxConfig() {
        }

        void initMatrixDimensions(bool matrix) {
            if (matrix) {
                this->matrixHeight = max(
                        this->matrixHeight, 1);
                this->matrixWidth = this->numPixels / this->matrixHeight;
                if (this->matrixWidth < 1) {
                    this->matrixWidth = 1;
                    this->matrixHeight = this->numPixels;
                }
            } else {
                this->matrixHeight = 1;
                this->matrixWidth = this->numPixels;
            }
        }

        uint8_t* getI2cPins() const {
            return i2cPins;
        }

        void setI2cPins(uint8_t* i2cPins = NULL) {
            this->i2cPins = i2cPins;
        }

        uint8_t* getI2cPwmPins() const {
            return i2cPWMPins;
        }

        void setI2cPwmPins(uint8_t* i2cPwmPins = NULL) {
            i2cPWMPins = i2cPwmPins;
        }

        bool isMatrix() const {
            return matrix;
        }

        void setMatrix(bool matrix) {
            this->matrix = matrix;
            initMatrixDimensions(
                    matrix);
        }

        uint8_t getMatrixHeight() const {
            return matrixHeight;
        }

        void setMatrixHeight(uint8_t matrixHeight) {
            this->matrixHeight = matrixHeight;
            initMatrixDimensions(
                    matrix);
        }

        uint8_t getMatrixWidth() const {
            return matrixWidth;
        }

        uint8_t* getNativePwmPins() const {
            return nativePWMPins;
        }

        void setNativePwmPins(uint8_t* nativePwmPins = NULL) {
            nativePWMPins = nativePwmPins;
        }

        uint8_t getNumColorsPerPixel() const {
            return numColorsPerPixel;
        }

        void setNumColorsPerPixel(uint8_t numColorsPerPixel) {
            this->numColorsPerPixel = numColorsPerPixel;
        }

        uint8_t getNumPixels() const {
            return numPixels;
        }

        void setNumPixels(uint8_t numPixels) {
            this->numPixels = numPixels;
        }

        uint8_t* getSpiPins() const {
            return spiPins;
        }

        void setSpiPins(uint8_t* spiPins = NULL) {
            this->spiPins = spiPins;
        }

        uint8_t getStripType() const {
            return stripType;
        }

        void setStripType(uint8_t stripType) {
            this->stripType = stripType;
        }
    };
}
#endif
