#include "storage.h"
#include "constants.h"

Storage::Storage() {
    status = ERROR_SD_UNINITIALIZED;
}


void Storage::setup() {
    SPI.setMOSI(SD_MOSI);
    SPI.setSCK(SD_SCK);
    SPI.setMISO(SD_MISO);
    SPI.setCS(SD_CS);

    currentFrame = 0;
    currentChunk = 0;

    if (!SD.begin(SD_CS)) {
        Serial.println("Card failed, or not present");
        status = ERROR_SD_MISSING;
        return;
    }
    for (uint8_t fileNum = 0; fileNum <= 255; fileNum++) {
        filename = String(fileNum) + ".log";
        if (!SD.exists(filename.c_str())) {
            break;
        }
        if (fileNum == 255) {
            status = ERROR_SD_TOO_MANY_LOGS;
            return;
        }
    }
    dataFile = SD.open(filename.c_str(), O_CREAT | O_WRITE);
    if (!dataFile) {
        status = ERROR_SD_UNWRITABLE;
        return;
    }
    status = ERROR_SD_OK;
}

uint8_t Storage::getStatus() {
    return status;
}

void Storage::addTime(uint32_t time) {
    cache.cache[currentFrame] &= ~TIME_MASK; // Zero out the time section
    uint64_t conv = (uint64_t) time;
    conv &= TIME_MASK;
    cache.cache[currentFrame] |= conv;
}

void Storage::addForce(uint32_t force) {
    cache.cache[currentFrame] &= ~FORCE_MASK; // Zero out the force section
    uint64_t conv = (uint64_t) force;
    conv <<= 16;
    conv &= FORCE_MASK; // Drop any value in the MSB of the word
    cache.cache[currentFrame] |= conv;
}

void Storage::addPressure(uint32_t pressure) {
    cache.cache[currentFrame] &= ~PRES_MASK; // Zero out the pressure section
    uint64_t conv = (uint64_t) pressure;
    conv <<= 40;
    conv &= PRES_MASK; // Shouldn't be required because the shift pushes it to the MSB
    cache.cache[currentFrame] |= conv;
}

void Storage::incrementFrame() {
    currentFrame++;
    if (currentFrame == NUM_FRAMES) {
        dataFile.write(cache.byteCache, NUM_FRAMES * 8);
        dataFile.flush();
        currentFrame = 0;
        currentChunk += 1;
    }
}

void Storage::dumpToSerial() {
    uint32_t lower = (uint32_t) cache.cache[currentFrame];
    uint32_t upper = (uint32_t) (cache.cache[currentFrame] >> 32);
    Serial.print(upper, HEX);
    Serial.println(lower, HEX);
}

uint64_t Storage::getFrame(uint16_t index) {
    return cache.cache[index];
}

uint16_t Storage::getNumFrames() {
    return 0; // UPDATE
}

uint16_t Storage::getCurrentFrame() {
    return currentFrame;
}
