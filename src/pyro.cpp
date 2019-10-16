#include "pyro.h"

PyroChannel::PyroChannel(uint8_t firingPin, uint8_t continuityPin, uint32_t fireDuration) {
    firePin = firingPin;
    contPin = continuityPin;
    duration = fireDuration;
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

void PyroChannel::fire() {
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
