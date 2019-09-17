#include "src/ADS1219.h"
#include "src/pyro.h"

ADS1219 adc(0x40, 19, 18);
PyroChannel pyro(5, 21, 100);

uint32_t lastReading = 0;
uint32_t lastFire = 0;

void setup() {
    adc.setup();
    pyro.setup();
    Serial.begin(9600);
    delay(250);
}

void loop() {
    adc.writeRegister(CONFIG_READ_LC);
    adc.requestReading();
    Serial.println(lastReading);
    if (millis() - lastFire > 1000) {
        pyro.fire();
        lastFire = millis();
    }
    pyro.update();
    lastReading = adc.waitForReading();
}