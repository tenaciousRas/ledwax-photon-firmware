#ifndef LEDWAX_CONFIG_H
#define LEDWAX_CONFIG_H
#include "ledwax_photon_constants.h"

namespace ledwaxconfig {
    /*
     class IPeriphPinDefs {
     public:
     virtual ~IPeriphPinDefs() {
     }
     virtual uint8_t getPeriphInterfaceTypeScalar();
     virtual uint8_t getNumWires();
     };

     class SPIPinDefs: public virtual IPeriphPinDefs {
     public:
     uint8_t numWires;
     uint8_t clkPin, misoPin, mosiPin, ssPin;
     // eclipse formatting break
     bool clkPinSet, misoPinSet, mosiPinSet, ssPinSet;

     SPIPinDefs(uint8_t numWires, int16_t clkPin = -1, int16_t mosiPin = -1, int16_t misoPin = -1,
     int16_t ssPin = -1) {
     this->numWires = numWires;
     if (-1 < clkPin) {
     this->clkPinSet = true;
     } else {
     this->clkPinSet = false;
     }
     this->clkPin = clkPin;
     if (-1 < misoPin) {
     this->misoPinSet = true;
     } else {
     this->misoPinSet = false;
     }
     this->misoPin = misoPin;
     if (-1 < mosiPin) {
     this->mosiPinSet = true;
     } else {
     this->mosiPinSet = false;
     }
     this->mosiPin = mosiPin;
     if (-1 < ssPin) {
     this->ssPinSet = true;
     } else {
     this->ssPinSet = false;
     }
     this->ssPin = ssPin;
     }

     ~SPIPinDefs() {
     }

     uint8_t getPeriphInterfaceTypeScalar() {
     return LWX_INTERFACE_TYPE_SPI;
     }

     uint8_t getNumWires() {
     return this->numWires;
     }
     };

     class I2CPinDefs: public virtual IPeriphPinDefs {
     public:
     uint8_t sckPin, sdaPin;
     // eclipse formatting break

     I2CPinDefs(uint8_t sckPin, uint8_t sdaPin) {
     this->sckPin = sckPin;
     this->sdaPin = sdaPin;
     }

     ~I2CPinDefs() {
     }

     uint8_t getPeriphInterfaceTypeScalar() {
     return LWX_INTERFACE_TYPE_I2C;
     }

     uint8_t getNumWires() {
     return 2;
     }
     };

     //    class ILEDWaxConfig {
     //    public:
     //        virtual ~ILEDWaxConfig() {
     //        }
     //        virtual uint8_t getStripType();
     //        virtual uint8_t getNumPixels();
     //        virtual uint8_t getNumColorsPerPixel();
     //        virtual bool isMatrix();
     //        virtual uint8_t getMatrixHeight();
     //        virtual void setMatrixHeight(uint8_t);
     //        virtual uint8_t getMatrixWidth();
     //        virtual void setMatrixWidth(uint8_t);
     //        virtual IPeriphPinDefs* getPinDefs();
     //    };
     */
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
            this->stripType = NULL;
            this->numPixels = NULL;
            this->numColorsPerPixel = NULL;
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

/*
 typedef struct {
 uint8_t stripType;
 uint8_t numPixels;
 uint8_t numColorsPerPixel;
 // formatting break
 bool matrix = false;
 uint8_t matrixHeight = 1;
 uint8_t matrixWidth;
 uint8_t *spiPins = NULL;
 uint8_t *i2cPins = NULL;
 uint8_t *i2cPWMPins = NULL;
 uint8_t *nativePWMPins = NULL;
 } ledwax_photon_strip_config;
 */
}
#endif
