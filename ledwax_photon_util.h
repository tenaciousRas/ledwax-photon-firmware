#ifndef LEDWAX_UTIL_H
#define LEDWAX_UTIL_H

#include "ledwax_photon_constants.h"

namespace ledwaxutil {
    class LEDWaxPhotonUtil {

    public:
        LEDWaxPhotonUtil();
        ~LEDWaxPhotonUtil();

        bool static startsWith(const char*, const char*);
        uint32_t rgbColor(uint8_t, uint8_t, uint8_t);
        uint32_t wheel(uint8_t);

    private:
    };
}
#endif
