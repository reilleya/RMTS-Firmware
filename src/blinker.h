#include <arduino.h>

typedef struct Pattern {
    uint32_t onDelay; // The amount of time the LED is on per blink. Measured in MS. If 0, the LED stays off.
    uint32_t offDelay; // The amount of time the LED spends off between sequences of blinks. Measured in MS. 
    uint32_t numBlinks; // The number of blinks in a sequence. Must be at least 1.
    uint32_t betweenBlinksDelay; // The amount of time between the blinks in a sequence. Measured in MS.
} BlinkPattern;

class Blinker {
    public:
        Blinker(uint8_t blinkPin);
        void setup();
        void update();
        void setPattern(BlinkPattern newPattern);
    
    private:
        void toggleState(bool newState);

        uint8_t ledPin;

        bool state;
        uint32_t blinks;
        uint32_t actionTime;

        BlinkPattern pattern;
};