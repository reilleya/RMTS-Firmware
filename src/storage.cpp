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
    uint64_t conv = (uint64_t) time;
    conv &= TIME_MASK;
    if (fillingA) {
        cacheA.cache[currentFrame] &= ~TIME_MASK; // Zero out the time section
        cacheA.cache[currentFrame] |= conv;
    } else {
        cacheB.cache[currentFrame] &= ~TIME_MASK; // Zero out the time section
        cacheB.cache[currentFrame] |= conv;
    }
}

void Storage::addForce(uint32_t force) {
    uint64_t conv = (uint64_t) force;
    conv <<= 16;
    conv &= FORCE_MASK; // Drop any value in the MSB of the word
    if (fillingA) {
        cacheA.cache[currentFrame] &= ~FORCE_MASK; // Zero out the force section
        cacheA.cache[currentFrame] |= conv;
    } else {
        cacheB.cache[currentFrame] &= ~FORCE_MASK; // Zero out the force section
        cacheB.cache[currentFrame] |= conv;
    }
}

void Storage::addPressure(uint32_t pressure) {
    uint64_t conv = (uint64_t) pressure;
    conv <<= 40;
    conv &= PRES_MASK; // Shouldn't be required because the shift pushes it to the MSB
    if (fillingA) {
        cacheA.cache[currentFrame] &= ~PRES_MASK; // Zero out the pressure section
        cacheA.cache[currentFrame] |= conv;
    } else {
        cacheB.cache[currentFrame] &= ~PRES_MASK; // Zero out the pressure section
        cacheB.cache[currentFrame] |= conv;
    }
}

bool Storage::incrementFrame() {
    totalFrames++;
    currentFrame++;
    if (currentFrame == NUM_FRAMES) {
        fillingA = !fillingA;
        writing = true;
        currentFrame = 0;
        return true;
    }
    return false;
}

void Storage::update() {
    if (writing) {
        bool old = fillingA;
        if (fillingA) {
            dataFile.write(cacheB.byteCache, NUM_FRAMES * 8); // Write B while we are filling A
        } else {
            dataFile.write(cacheA.byteCache, NUM_FRAMES * 8); // Write A while B is filling
        }
        dataFile.flush();
        writing = false;
    }
}

void Storage::dumpToSerial() {
    uint32_t lower = (uint32_t) cacheA.cache[currentFrame]; // UPDATE
    uint32_t upper = (uint32_t) (cacheA.cache[currentFrame] >> 32); // UPDATE
    Serial.print(upper, HEX);
    Serial.println(lower, HEX);
}

uint64_t Storage::getFrame(uint16_t index) {
    return cacheA.cache[index]; // UPDATE
}

uint64_t Storage::getTotalFrames() {
    return totalFrames;
}

uint16_t Storage::getCurrentFrame() {
    return currentFrame;
}
