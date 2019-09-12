#include "ads1219.h"

ADS1219::ADS1219(uint8_t address, uint8_t dataReadyPin, uint8_t ampPin) {
    addr = address;
    drdy = dataReadyPin;
    amp = ampPin;
}

void ADS1219::setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_16_17, I2C_PULLUP_EXT, 400000);
    Wire.setDefaultTimeout(10000);
    pinMode(drdy, INPUT);
    pinMode(amp, OUTPUT);
    digitalWrite(amp, HIGH);
}

void ADS1219::writeRegister(uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(CONFIG_ADDRESS);
    Wire.write(value);
    Wire.endTransmission();
}

void ADS1219::requestReading() {
    Wire.beginTransmission(addr);
    Wire.write(0x08);
    Wire.endTransmission();
}

uint32_t ADS1219::getReading() {
    Wire.beginTransmission(addr);
    Wire.write(0x10);
    Wire.endTransmission();
    Wire.requestFrom(addr, (uint8_t) 3);
    uint32_t output = Wire.read();
    output <<= 8;
    output |= Wire.read();
    output <<= 8;
    output |= Wire.read();
    return output;
}

uint32_t ADS1219::waitForReading() {
    while (digitalRead(drdy) == HIGH);
    return getReading();
}
