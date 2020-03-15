#include <arduino.h>
#include <SD.h>
#include <SPI.h>

#define TIME_MASK  0x000000000000FFFF
#define FORCE_MASK 0x000000FFFFFF0000
#define PRES_MASK  0xFFFFFF0000000000

#define NUM_FRAMES 64

typedef union storageCacheUnion {
  uint64_t cache[NUM_FRAMES];
  char byteCache[NUM_FRAMES * 8];
} cacheUnion;

class Storage {
    public:
        Storage();
        void setup();
        uint8_t getStatus();

        void addTime(uint32_t time);
        void addForce(uint32_t force);
        void addPressure(uint32_t pressure);
        void incrementFrame();
        uint16_t getCurrentFrame();

        void update();

        void processData();
        void dumpToSerial();
        uint64_t getFrame(uint16_t index);
        uint16_t getNumFrames();

    private:
        void writeChunk();

        uint8_t status;

        String filename;
        File dataFile;

        uint16_t currentFrame;
        uint16_t currentChunk;

        cacheUnion cacheA;
        cacheUnion cacheB;
        bool fillingA = true;

        bool writing = false;
};
