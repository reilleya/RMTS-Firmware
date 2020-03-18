#include <arduino.h>

class PyroChannel {
    public:
        PyroChannel(uint8_t firingPin, uint8_t continuityPin);
        void setup();
        void update();
        void fire(uint32_t fireDuration);
        void stopFiring();
        bool getContinuity();
    
    private:
        uint8_t firePin;
        uint8_t contPin;
        uint32_t duration;

        bool firing;
        bool fired;
        uint32_t startedAt;
};