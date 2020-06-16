#include <arduino.h>
#include <SD.h>
#include <SPI.h>

#define TIME_MASK  0x000000000000FFFF
#define FORCE_MASK 0x000000FFFFFF0000
#define PRES_MASK  0xFFFFFF0000000000

#define NUM_FRAMES 64
#define FRAME_SIZE sizeof(uint64_t)

typedef union storageCacheUnion {
  uint64_t cache[NUM_FRAMES];
  char byteCache[NUM_FRAMES * FRAME_SIZE];
} cacheUnion;

typedef union storageFrameUnion {
  uint64_t frame;
  char bytes[8];
} frameUnion;


class Storage {
    public:
        Storage();
        void setup();
        uint8_t getStatus();

        void addTime(uint32_t time);
        void addForce(uint32_t force);
        void addPressure(uint32_t pressure);
        uint32_t getLastTime();
        uint32_t getLastForce();
        uint32_t getLastPressure();
        bool incrementFrame();
        uint64_t getTotalFrames();
        void update();

        void switchToResults();
        uint64_t getReadFrameIndex();
        uint64_t getFrame();

    private:
        uint8_t status;

        String filename;
        File dataFile;

        uint16_t currentFrame;
        uint64_t totalFrames;

        uint32_t lastTime;
        uint32_t lastForce;
        uint32_t lastPressure;

        cacheUnion cacheA;
        cacheUnion cacheB;
        bool fillingA = true;

        volatile bool writing = false;

        uint64_t resultsOffset = 0;
};
