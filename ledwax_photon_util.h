#ifndef LEDWAX_UTIL_H
#define LEDWAX_UTIL_H

#include "ledwax_photon_constants.h"

namespace ledwaxutil {
    class LEDWaxPhotonUtil {

    public:
        LEDWaxPhotonUtil();
        ~LEDWaxPhotonUtil();

        bool startsWith(const char*, const char*),
                isAddressableStrip(uint8_t stripNum);
        uint32_t rgbColor(uint8_t, uint8_t, uint8_t);
        uint32_t wheel(uint16_t);

    private:
    };
}
#endif
