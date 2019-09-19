#include "src/ADS1219.h"
#include "src/pyro.h"
#include "src/storage.h"

ADS1219 adc(0x40, 19, 18);
PyroChannel pyro(5, 21, 100);
Storage store;

uint32_t startTime = 0;

bool finished = false;
uint32_t forceReading = 0;
uint32_t presReading = 0;

void setup() {
    Serial.begin(9600);
    Serial1.begin(9600);
    adc.setup();
    pyro.setup();
    store.setup();

    delay(250);
}

void loop() {
    while (startTime == 0) {
        if (Serial1.available() > 0) {
            pyro.fire();
            startTime = millis();
        }
    }
    Serial.println("Reading now!");
    while (!finished) {
        adc.writeRegister(CONFIG_READ_LC);
        adc.requestReading();

        store.addTime(millis() - startTime);
        pyro.update();

        forceReading = adc.waitForReading();
        adc.writeRegister(CONFIG_READ_DUCER);
        adc.requestReading();

        store.addForce(forceReading);

        presReading = adc.waitForReading();
        store.addPressure(presReading);
        //store.dumpToSerial();

        finished = store.incrementFrame();
    }

    store.dumpToSD();

    while (finished) {
        Serial.println("Done!");
        delay(100);
    }
}