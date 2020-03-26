#include "blinker.h"
#include "constants.h"

Blinker::Blinker(uint8_t blinkPin) {
    ledPin = blinkPin;

    pattern = PATTERN_OFF;

    state = false;
    blinks = 0;
}

void Blinker::setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    state = false;
    blinks = 0;
    actionTime = millis();
}

void Blinker::update() {
    if (millis() > actionTime) {
        if (state) {
            toggleState(false);
            blinks++;
            if (blinks == pattern.numBlinks) {
                actionTime = millis() + pattern.offDelay;
                blinks = 0;
            } else {
                actionTime = millis() + pattern.betweenBlinksDelay;
            }
        } else {
            if (pattern.onDelay != 0) toggleState(true);
            actionTime = millis() + pattern.onDelay;
        }
    }
}

void Blinker::setPattern(BlinkPattern newPattern) {
    pattern = newPattern;
    actionTime = millis();
    toggleState(false);
    blinks = 0;
}

void Blinker::toggleState(bool newState) {
    digitalWrite(ledPin, newState);
    state = newState;
}
