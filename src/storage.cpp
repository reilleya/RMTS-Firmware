#include "storage.h"
#include "constants.h"

Storage::Storage() {
    status = ERROR_SD_UNINITIALIZED;
    lastForce = 0;
    lastPressure = 0;
    lastTime = 0;
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
    lastTime = time;
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
    lastForce = force;
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
    lastPressure = pressure;
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

uint32_t Storage::getLastTime() {
    return lastTime;
}

uint32_t Storage::getLastForce() {
    return lastForce;
}

uint32_t Storage::getLastPressure() {
    return lastPressure;
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

uint64_t Storage::getTotalFrames() {
    return totalFrames;
}

void Storage::update() {
    if (writing) {
        writing = false; // Set this now so we don't accidentally ignore a write that is requested during this one
        if (fillingA) {
            dataFile.write(cacheB.byteCache, NUM_FRAMES * FRAME_SIZE); // Write B while we are filling A
        } else {
            dataFile.write(cacheA.byteCache, NUM_FRAMES * FRAME_SIZE); // Write A while B is filling
        }
        dataFile.flush();
    }
}

void Storage::switchToResults() {
    dataFile.close();
    dataFile = SD.open(filename.c_str(), O_RDONLY);
    currentFrame = 0;
}

uint64_t Storage::getReadFrameIndex() {
    return currentFrame;
}

uint64_t Storage::getFrame() {
    frameUnion retData;

    dataFile.read(retData.bytes, FRAME_SIZE);
    currentFrame += RESULTS_STRIDE;
    if (currentFrame >= totalFrames) {
        resultsOffset = (resultsOffset + RESULTS_OFFSET_INCREMENT) % RESULTS_STRIDE;
        currentFrame = resultsOffset;
    }
    dataFile.seek(currentFrame * FRAME_SIZE);

    return retData.frame;
}
