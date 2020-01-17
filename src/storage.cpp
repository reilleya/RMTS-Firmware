#include "storage.h"
#include "constants.h"

Storage::Storage() {
    status = ERROR_SD_UNINITIALIZED;
    lastFiringFrame = 0;
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
        filename = String(fileNum) + ".txt";
        if (!SD.exists(filename.c_str())) {
            break;
        }
        if (fileNum == 255) {
            status = ERROR_SD_TOO_MANY_LOGS;
            return;
        }
    }
    dataFile = SD.open(filename.c_str(), FILE_WRITE);
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
    cache[currentFrame] &= ~TIME_MASK; // Zero out the time section
    uint64_t conv = (uint64_t) time;
    conv &= TIME_MASK;
    cache[currentFrame] |= conv;
}

void Storage::addForce(uint32_t force) {
    cache[currentFrame] &= ~FORCE_MASK; // Zero out the force section
    uint64_t conv = (uint64_t) force;
    conv <<= 16;
    conv &= FORCE_MASK; // Drop any value in the MSB of the word
    cache[currentFrame] |= conv;
}

void Storage::addPressure(uint32_t pressure) {
    cache[currentFrame] &= ~PRES_MASK; // Zero out the pressure section
    uint64_t conv = (uint64_t) pressure;
    conv <<= 40;
    conv &= PRES_MASK; // Shouldn't be required because the shift pushes it to the MSB
    cache[currentFrame] |= conv;
}

bool Storage::incrementFrame() {
    currentFrame++;
    return currentFrame >= NUM_FRAMES;
}

void Storage::processData() {
    uint32_t maxForce = 0;
    uint32_t force;
    for (uint16_t i = 0; i < NUM_FRAMES; i++) {
        force = (cache[i] & FORCE_MASK) >> 16;
        if (force > maxForce) maxForce = force;
    }
    for (uint16_t i = NUM_FRAMES; i > 0; i--) {
        force = (cache[i] & FORCE_MASK) >> 16;
        if (force * 100 > maxForce) {
            lastFiringFrame = i;
            break;
        }
    }
}

void Storage::dumpToSerial() {
    uint32_t lower = (uint32_t) cache[currentFrame];
    uint32_t upper = (uint32_t) (cache[currentFrame] >> 32);
    Serial.print(upper, HEX);
    Serial.println(lower, HEX);
}

uint64_t Storage::getFrame(uint16_t index) {
    return cache[index];
}

uint16_t Storage::getNumFrames() {
    return lastFiringFrame;
}

uint16_t Storage::getCurrentFrame() {
    return currentFrame;
}

void Storage::dumpToSD() {
    for (uint16_t index = 0; index < NUM_FRAMES; index++) {
        uint32_t time = cache[index] & TIME_MASK;
        uint32_t force = (cache[index] & FORCE_MASK) >> 16;
        uint32_t pressure = (cache[index] & PRES_MASK) >> 40;

        dataFile.print(time);
        dataFile.print(",");
        dataFile.print(force);
        dataFile.print(",");
        dataFile.println(pressure);
    }
    dataFile.close();
}
