#include "src/ADS1219.h"
#include "src/pyro.h"
#include "src/storage.h"
#include "src/radio.h"
#include "src/constants.h"
#include "src/blinker.h"

ADS1219 adc(ADC_ADDRESS, ADC_DRDY_PIN, ADC_AMP_PIN);
PyroChannel pyro(PYRO_FIRE_PIN, PYRO_CONT_PIN);
Storage store;
RadioHandler radio;
Blinker status(STATUS_PIN);

systemState sysState = SETUP;

// Setup state locals
uint32_t setupForceReading = 0;
uint32_t setupPresReading = 0;
bool extFireTriggered = false;

// Error state locals

// Firing state locals
bool forceReq = true; // Which kind of sensor reading the interrupt is waiting on. If false, it must have been pressure.
bool recordingCanceled = false; // Set when the main loop gets a stop packet. Used by the interrupt to know if it should keep looping.
bool doneRecording = false; // Set by the interrupt when it collects its final measurement.
uint32_t firingStateStarted; // The time that the firing state was entered.
uint32_t firingDuration;
uint32_t currentTime;
bool firingSkipWriting = true; // This is kind of a bandaid. The first value read from the ADC is wrong so we don't bother writing it.
bool firingWritingSD = false;
uint32_t firingForceReading;
uint32_t firingPressureReading;

// Results state locals
uint16_t resultsVersionPacketCounter = 0;

void dataReady() {
    cli();
    if (sysState != FIRING) {
        sei();
        return;
    }
    if (forceReq) {
        firingForceReading = adc.getReading();
        if (!firingSkipWriting) {
            store.addForce(firingForceReading);
            store.addTime(millis() - firingStateStarted);
        }
        forceReq = false;
        adc.writeRegister(CONFIG_READ_DUCER);
        adc.requestReading();
    } else {
        firingPressureReading = adc.getReading();
        if (!firingSkipWriting) {
            store.addPressure(firingPressureReading);
            firingWritingSD = store.incrementFrame();
        } else {
            firingWritingSD = false;
            firingSkipWriting = false;
        }
        if (recordingCanceled && firingWritingSD) {
            doneRecording = true;
        } else {
            forceReq = true;
            adc.writeRegister(CONFIG_READ_LC);
            adc.requestReading();
        }
    }
    sei();
}

void extFire() {
    cli();
    if (sysState != SETUP) {
        sei();
        return;
    }
    detachInterrupt(digitalPinToInterrupt(TRIGGER_PIN));
    extFireTriggered = true;
    sei();
}

void setupStateUpdate() {
    if (hasError()) {
        sysState = ERROR;
        status.setPattern(PATTERN_ERROR);
        Serial.println("Entering error state");
        return;
    }

    if (extFireTriggered) {
        enterFiringState();
        return;
    }

    adc.writeRegister(CONFIG_READ_LC_SETUP);
    adc.requestReading();

    radio.update();
    status.update();

    setupForceReading = adc.waitForReading();

    adc.writeRegister(CONFIG_READ_DUCER_SETUP);
    adc.requestReading();
    
    radio.sendVersionPacket();

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

    while (radio.available()) {
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
    status.setPattern(PATTERN_ON);
    status.update();
    firingStateStarted = millis();
    adc.writeRegister(CONFIG_READ_DUCER);
    adc.requestReading();
    radio.resetBuffers();
}

void firingStateUpdate() {

    store.update();
    status.update();

    if (store.getTotalFrames() == NUM_CAL_FRAMES && !extFireTriggered) pyro.fire(firingDuration);
    pyro.update();

    radio.update();
    while (radio.available()) {
        packet pack = radio.readPacket();
        if (pack.type == PACKET_STOP && !recordingCanceled) {
            recordingCanceled = true;
            Serial.println("Received stop packet");
            break;
        }
    }
    if (doneRecording) {
        enterFinishedState();
        return;
    }

    uint32_t lastTime = store.getLastTime();
    uint32_t lastForce = store.getLastForce();
    uint32_t lastPressure = store.getLastPressure();

    packet pack;
    pack.type = PACKET_FIRING;
    pack.seqNum = 0;
    pack.payload[0] = (uint8_t) lastForce;
    pack.payload[1] = (uint8_t) (lastForce >> 8);
    pack.payload[2] = (uint8_t) (lastForce >> 16);
    pack.payload[3] = (uint8_t) lastPressure;
    pack.payload[4] = (uint8_t) (lastPressure >> 8);
    pack.payload[5] = (uint8_t) (lastPressure >> 16);
    pack.payload[6] = (uint8_t) lastTime;
    pack.payload[7] = (uint8_t) (((uint8_t) (lastTime >> 8)) << 1) + pyro.getContinuity();

    radio.sendPacket(pack);
}

void enterFinishedState() {
    sysState = FINISHED;
    status.setPattern(PATTERN_FINISHED);
    store.update(); // To ensure a final write takes place
    store.switchToResults();
    Serial.println("Entering finished state");
}

void finishedStateUpdate() {
    status.update();
    packet pack;
    pack.type = PACKET_RESULTS;
    pack.seqNum = store.getReadFrameIndex();
    uint64_t frame = store.getFrame();
    memcpy(pack.payload, &frame, sizeof(uint64_t));
    radio.sendPacket(pack);

    resultsVersionPacketCounter++;
    if (resultsVersionPacketCounter == RESULTS_VERSION_PERIOD) {
        resultsVersionPacketCounter = 0;
        radio.sendVersionPacket();
    }
}

bool hasError() {
    return store.getStatus() != ERROR_SD_OK or adc.getStatus() != ERROR_ADC_OK;
}

void errorStateUpdate() {
    status.update();

    radio.sendVersionPacket();

    packet pack;
    pack.type = PACKET_ERROR;
    pack.seqNum = 0;
    pack.payload[0] = store.getStatus();
    pack.payload[1] = adc.getStatus();
    radio.sendPacket(pack);

    delay(5);
}


void setup() {
    pyro.setup();
    Serial.begin(9600);
    adc.setup();
    store.setup();
    radio.setup();
    status.setup();

    attachInterrupt(adc.drdy, dataReady, FALLING);

    pinMode(TRIGGER_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), extFire, RISING);

    status.setPattern(PATTERN_SETUP);

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
