#include "src/ADS1219.h"
#include "src/pyro.h"
#include "src/storage.h"
#include "src/radio.h"

ADS1219 adc(0x40, 19, 18);
PyroChannel pyro(5, 21, 100);
Storage store;
RadioHandler radio;

uint32_t startTime = 0;

bool finished = false;
uint32_t forceReading = 0;
uint32_t presReading = 0;

void setup() {
    Serial.begin(9600);
    adc.setup();
    pyro.setup();
    store.setup();
    radio.setup();

    delay(250);
}

void loop() {
    while (true) {
        adc.writeRegister(CONFIG_READ_DUCER_SETUP);
        adc.requestReading();
        forceReading = adc.waitForReading();
        radio.update();
        packet pack;
        pack.type = 0;
        pack.seqNum = 0;
        pack.payload[0] = (uint8_t) forceReading;
        pack.payload[1] = (uint8_t) (forceReading >> 8);
        pack.payload[2] = (uint8_t) (forceReading >> 16);
        pack.payload[3] = 0;
        pack.payload[4] = 0;
        pack.payload[5] = 0;
        pack.payload[6] = 0;
        pack.payload[7] = 210;
        radio.sendPacket(pack);
        /*if (radio.available() > 0) {
            packet pack = radio.readPacket();
            Serial.println(pack.type);
            Serial.println(pack.seqNum);
            Serial.println("-------");
        }*/
        delay(100);
    }

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

        finished = store.incrementFrame();
    }

    store.dumpToSD();

    while (finished) {
        Serial.println("Done!");
        delay(100);
    }
}