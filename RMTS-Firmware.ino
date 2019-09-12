#include "src/ADS1219.h"

ADS1219 adc(0x40, 19, 18);

uint32_t lastReading = 0;

void setup() {
    adc.setup();
    Serial.begin(9600);
    delay(250);
}

void loop() {
    adc.writeRegister(CONFIG_READ_LC);
    adc.requestReading();
    Serial.println(lastReading);
    lastReading = adc.waitForReading();
}