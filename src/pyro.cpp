#include "pyro.h"

PyroChannel::PyroChannel(uint8_t firingPin, uint8_t continuityPin) {
    firePin = firingPin;
    contPin = continuityPin;
    firing = false;
    fired = false;
}

void PyroChannel::setup() {
    pinMode(firePin, OUTPUT);
    pinMode(contPin, INPUT);
    digitalWrite(firePin, LOW);
}

void PyroChannel::update() {
    if (firing) {
        if (millis() - startedAt > duration) {
            stopFiring();
        }
    }
}

void PyroChannel::fire(uint32_t fireDuration) {
    if (!fired) { // Never fire more than once per power-on
        duration = fireDuration;
        digitalWrite(firePin, HIGH);
        startedAt = millis();
        firing = true;
        fired = true;
    }
}

void PyroChannel::stopFiring() {
    digitalWrite(firePin, LOW);
    firing = false;
}

bool PyroChannel::getContinuity() {
    return digitalRead(contPin);
}
