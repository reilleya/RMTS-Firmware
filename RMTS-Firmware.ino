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
bool forceReq = true; // Which kind of sensor reading the interrupt is waiting on. If false, it must have been pressure.
bool recordingCanceled = false; // Set when the main loop gets a stop packet. Used by the interrupt to know if it should keep looping.
bool doneRecording = false; // Set by the interrupt when it collects its final measurement.
uint32_t firingStateStarted; // The time that the firing state was entered.
uint32_t firingDuration;
uint32_t currentTime;

// Results state locals
uint16_t resultsOffset = 0;

void dataReady() {
    cli();
    if (sysState != FIRING) {
        sei();
        return;
    }
    if (forceReq) {
        store.addForce(adc.getReading());
        store.addTime(millis() - firingStateStarted);
        forceReq = false;
        adc.writeRegister(CONFIG_READ_DUCER);
        adc.requestReading();
    } else {
        store.addPressure(adc.getReading());
        forceReq = true;
        bool writing = store.incrementFrame();
        if (recordingCanceled && writing) {
            doneRecording = true;
        } else {
            adc.writeRegister(CONFIG_READ_LC);
            adc.requestReading();
        }
    }
    sei();
}


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
    radio.sendPacket(pack);

    while (radio.available() > 0) {
        packet pack = radio.readPacket();
        if (pack.type == PACKET_FIRE) {
            firingDuration = (uint16_t) pack.payload[2] + ((uint16_t) pack.payload[3] << 8);
            enterFiringState();
            break;
        }
        if (pack.type == PACKET_CAL_START) {
            Serial.println("Starting cal");
            adc.startCal();
        }
        if (pack.type == PACKET_CAL_STOP) {
            Serial.println("Stopping cal");
            adc.endCal();
        }
    }
}

void enterFiringState() {
    Serial.print("Entering fire state, firing for ");
    Serial.print(firingDuration);
    Serial.println(" ms");
    sysState = FIRING;
    firingStateStarted = millis();
    adc.writeRegister(CONFIG_READ_DUCER);
    adc.requestReading();
}

void firingStateUpdate() {

    store.update();

    if (store.getTotalFrames() == NUM_CAL_FRAMES) pyro.fire(firingDuration);
    pyro.update();

    radio.update();
    while (radio.available() > 0) {
        packet pack = radio.readPacket();
        if (pack.type == PACKET_STOP && !recordingCanceled) {
            recordingCanceled = true;
            Serial.println("Received stop packet");
            break;
        }
    }
    if (doneRecording) enterFinishedState();
}

void enterFinishedState() {
    sysState = FINISHED;
    store.update(); // To ensure a final write takes place
    store.switchToResults();
    Serial.println("Entering finished state");
}

void finishedStateUpdate() {
    packet pack;
    pack.type = PACKET_RESULTS;
    pack.seqNum = store.getReadFrameIndex();
    uint64_t frame = store.getFrame();
    memcpy(pack.payload, &frame, sizeof(uint64_t));
    radio.sendPacket(pack);
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
    radio.sendPacket(pack);

    delay(100);
}


void setup() {
    Serial.begin(9600);
    adc.setup();
    pyro.setup();
    store.setup();
    radio.setup();

    attachInterrupt(adc.drdy, dataReady, FALLING);

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
