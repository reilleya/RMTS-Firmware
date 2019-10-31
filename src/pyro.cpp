#include "pyro.h"

PyroChannel::PyroChannel(uint8_t firingPin, uint8_t continuityPin) {
    firePin = firingPin;
    contPin = continuityPin;
    firing = false;
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
    duration = fireDuration;
    digitalWrite(firePin, HIGH);
    startedAt = millis();
    firing = true;
}

void PyroChannel::stopFiring() {
    digitalWrite(firePin, LOW);
    firing = false;
}

bool PyroChannel::getContinuity() {
    return digitalRead(contPin);
}
