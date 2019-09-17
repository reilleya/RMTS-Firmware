#include <arduino.h>
#include "TeensyDelay.h"

class PyroChannel {
    public:
        PyroChannel(uint8_t firingPin, uint8_t continuityPin, uint32_t duration);
        void setup();
        void update();
        void fire();
        void stopFiring();
        bool getContinuity();
    
    private:
        uint8_t firePin;
        uint8_t contPin;
        uint32_t duration;

        bool firing;
        uint32_t startedAt;
};