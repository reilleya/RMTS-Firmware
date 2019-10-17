#include "radio.h"

RadioHandler::RadioHandler() {
    serialBufferIndex = 0;
    packetBufferIndex = 0;   
}

void RadioHandler::setup() {
    Serial1.begin(9600);
}

void RadioHandler::update() {
    while (Serial1.available() > 0) {
        uint8_t b = Serial1.read();
        if (b == RADIO_PREABLE_0 and not escape) {
            inPreamble = true;
            inPacket = false;
        }
        else if (b == RADIO_PREABLE_1 and not escape and inPreamble) {
            inPacket = true;
            serialBufferIndex = 0;
        }
        else if (inPacket and (b != RADIO_ESCAPE or escape)) {
            packetConv.data[serialBufferIndex] = b;
            serialBufferIndex++;
            if (serialBufferIndex == sizeof(packet)) {
                if (validatePacket(&packetConv.pack)) {
                    serialBufferIndex = 0;
                    packetBuffer[packetBufferIndex] = packetConv.pack;
                    packetBufferIndex++;
                }
                serialBufferIndex = 0;
                inPacket = false;
            }
        }
        escape = b == RADIO_ESCAPE;
    }
}

void RadioHandler::resetBuffers() {
    Serial1.clear();
    serialBufferIndex = 0;
    resetPacketBuffer();
}

bool RadioHandler::validatePacket(packet* testPack) {
    uint8_t total = testPack->type + testPack->seqNum + testPack->checksum;
    for (uint8_t i = 0; i < PACKET_PAYLOAD_LENGTH; i++) {
        total += testPack->payload[i];
    }
    return total == 0;
}

uint8_t RadioHandler::available() {
    return packetBufferIndex;
}

packet RadioHandler::readPacket() {
    packet retPacket = packetBuffer[0];
    for (uint8_t i = 0; i < packetBufferIndex - 1; i++) {
        packetBuffer[i] = packetBuffer[i + 1];
    }
    packetBufferIndex--;
    return retPacket;
}

void RadioHandler::resetPacketBuffer() {
    packetBufferIndex = 0;
}

void RadioHandler::sendPacket(packet toSend) {
    packetConv.pack = toSend;
    packetConv.pack.checksum = 0;
    uint8_t total = 0;
    for (uint8_t i = 0; i < sizeof(packet); i++) {
        total += packetConv.data[i];
    }
    packetConv.pack.checksum = 0 - total;
    Serial1.write(RADIO_PREABLE_0);
    Serial1.write(RADIO_PREABLE_1);
    for (uint8_t i = 0; i < sizeof(packet); i++) {
        if (packetConv.data[i] == RADIO_PREABLE_0 || packetConv.data[i] == RADIO_PREABLE_1 || packetConv.data[i] == RADIO_ESCAPE) {
            Serial1.write(RADIO_ESCAPE);
        }
        Serial1.write(packetConv.data[i]);
    }
}
