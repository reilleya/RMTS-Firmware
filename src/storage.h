#include <arduino.h>
#include <SD.h>
#include <SPI.h>

#define TIME_MASK  0x000000000000FFFF
#define FORCE_MASK 0x000000FFFFFF0000
#define PRES_MASK  0xFFFFFF0000000000

#define NUM_FRAMES 1000

class Storage {
    public:
        Storage();
        void setup();
        void addTime(uint32_t time);
        void addForce(uint32_t force);
        void addPressure(uint32_t pressure);
        bool incrementFrame();

        void dumpToSerial();
        void dumpToSD();

    private:
        uint16_t currentFrame;
        uint64_t cache[NUM_FRAMES];
};
