#include "storage.h"
#include "constants.h"

Storage::Storage() {

}


void Storage::setup() {
    SPI.setMOSI(SD_MOSI);
    SPI.setSCK(SD_SCK);
    SPI.setMISO(SD_MISO);
    SPI.setCS(SD_CS);

    currentFrame = 0;

    if (!SD.begin(SD_CS)) {
        Serial.println("Card failed, or not present");
    }
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

void Storage::dumpToSerial() {
    uint32_t lower = (uint32_t) cache[currentFrame];
    uint32_t upper = (uint32_t) (cache[currentFrame] >> 32);
    Serial.print(upper, HEX);
    Serial.println(lower, HEX);
}

void Storage::dumpToSD() {
    dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile) {
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
    else {
        Serial.println("Unable to make file!");
    }
}
