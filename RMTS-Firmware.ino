#include "src/ADS1219.h"
#include "src/pyro.h"
#include "src/storage.h"
#include "src/radio.h"
#include "src/constants.h"

ADS1219 adc(ADC_ADDRESS, ADC_DRDY_PIN, ADC_AMP_PIN);
PyroChannel pyro(PYRO_FIRE_PIN, PYRO_CONT_PIN);
Storage store;
RadioHandler radio;


systemState sysState = SETUP;

// Setup state locals
uint32_t setupForceReading = 0;
uint32_t setupPresReading = 0;

// Error state locals

// Firing state locals
uint32_t firingStateStarted;

uint32_t recordingDuration;
uint32_t currentTime;

// Results state locals
uint16_t resultsOffset = 0;


void setupStateUpdate() {
    if (hasError()) {
        sysState = ERROR;
        Serial.println("Entering error state");
        return;
    }

    adc.writeRegister(CONFIG_READ_LC_SETUP);
    adc.requestReading();

    radio.update();

    setupForceReading = adc.waitForReading();

    adc.writeRegister(CONFIG_READ_DUCER_SETUP);
    adc.requestReading();
    
    setupPresReading = adc.waitForReading();

    packet pack;
    pack.type = PACKET_SETUP;
    pack.seqNum = 0;
    pack.payload[0] = (uint8_t) setupForceReading;
    pack.payload[1] = (uint8_t) (setupForceReading >> 8);
    pack.payload[2] = (uint8_t) (setupForceReading >> 16);
    pack.payload[3] = (uint8_t) setupPresReading;
    pack.payload[4] = (uint8_t) (setupPresReading >> 8);
    pack.payload[5] = (uint8_t) (setupPresReading >> 16);
    pack.payload[6] = (uint8_t) pyro.getContinuity();
    pack.payload[7] = 0;
    radio.sendPacket(pack);

    while (radio.available() > 0) {
        packet pack = radio.readPacket();
        if (pack.type == PACKET_FIRE) {
            recordingDuration = (uint16_t) pack.payload[0] + ((uint16_t) pack.payload[1] << 8);
            Serial.print("Entering fire state for ");
            Serial.print(recordingDuration);
            Serial.print(" ms");
            sysState = FIRING;
            firingStateStarted = millis();
            pyro.fire(100);
            break;
        }
    }
}


void firingStateUpdate() {
    adc.writeRegister(CONFIG_READ_LC);
    adc.requestReading();

    currentTime = millis() - firingStateStarted;
    store.addTime(currentTime);
    pyro.update();

    setupForceReading = adc.waitForReading();
    adc.writeRegister(CONFIG_READ_DUCER);
    adc.requestReading();

    store.addForce(setupForceReading);
    radio.resetBuffers();

    setupPresReading = adc.waitForReading();
    store.addPressure(setupPresReading);

    if (store.incrementFrame() || currentTime > recordingDuration) { // Returns true if the buffer is full
        sysState = FINISHED;
        Serial.println("Entering finished state");
        store.dumpToSD();
        store.processData();
    }
}


void finishedStateUpdate() {
    uint64_t frame = 0;
    for(uint16_t i = resultsOffset; i < store.getNumFrames(); i += 10) {
        packet pack;
        pack.type = PACKET_RESULTS;
        pack.seqNum = i;
        frame = store.getFrame(i);
        memcpy(pack.payload, &frame, sizeof(uint64_t));
        radio.sendPacket(pack);
    }
    resultsOffset += 1;
    if (resultsOffset == 10) resultsOffset = 0;
}

bool hasError() {
    return store.getStatus() != ERROR_SD_OK or adc.getStatus() != ERROR_ADC_OK;
}

void errorStateUpdate() {
    packet pack;
    pack.type = PACKET_ERROR;
    pack.seqNum = 0;
    pack.payload[0] = store.getStatus();
    pack.payload[1] = adc.getStatus();
    pack.payload[2] = 0;
    pack.payload[3] = 0;
    pack.payload[4] = 0;
    pack.payload[5] = 0;
    pack.payload[6] = 0;
    pack.payload[7] = 0;
    radio.sendPacket(pack);

    delay(100);
}


void setup() {
    Serial.begin(9600);
    adc.setup();
    pyro.setup();
    store.setup();
    radio.setup();

    delay(250);
}

void loop() {
    switch (sysState) {
        case SETUP:
            setupStateUpdate();
            break;
        case FIRING:
            firingStateUpdate();
            break;
        case FINISHED:
            finishedStateUpdate();
            break;
        case ERROR:
            errorStateUpdate();
            break;
    }
}
