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
        serialBuffer[serialBufferIndex] = Serial1.read();
        serialBufferIndex++;
        if (serialBufferIndex == SERIAL_BUFFER_LENGTH) { // Reset serial buffer if it fills
            serialBufferIndex = 0;
        }
    }
    if (serialBufferIndex >= sizeof(packet)) {
        memcpy(&packetConv.data, &serialBuffer + serialBufferIndex - sizeof(packet), sizeof(packet));
        if (validatePacket(&packetConv.pack)) {
            Serial.println("Valid packet!");
            serialBufferIndex = 0;
            packetBuffer[packetBufferIndex] = packetConv.pack;
            packetBufferIndex++;
        }
        else {
            Serial.println("Invalid packet!");
        }
    }
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
    uint8_t total = toSend.type + toSend.seqNum;
    for (uint8_t i = 0; i < PACKET_PAYLOAD_LENGTH; i++) {
        total += toSend.payload[i];
    }
    toSend.checksum = 0 - total;
    packetConv.pack = toSend;
    for (uint8_t i = 0; i < sizeof(packet); i++) {
        Serial1.write(packetConv.data[i]);
    }
}
