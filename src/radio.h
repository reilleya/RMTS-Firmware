#include <arduino.h>

#define PACKET_PAYLOAD_LENGTH 8

#define SERIAL_BUFFER_LENGTH 128

#define PACKET_BUFFER_LENGTH 8

typedef struct radioPacket {
    uint8_t type;
    uint8_t seqNum;
    uint8_t payload[PACKET_PAYLOAD_LENGTH];
    uint8_t checksum;
} packet;

typedef union radioPacketUnion {
  packet pack;
  uint8_t data[sizeof(packet)];
} packetUnion;

class RadioHandler {
    public:
        RadioHandler();
        void setup();
        void update();
    
        bool validatePacket(packet* testPack);

        uint8_t available();
        packet readPacket();
        void resetPacketBuffer();

        void sendPacket(packet toSend);

    private:
        uint8_t serialBuffer[SERIAL_BUFFER_LENGTH];
        uint8_t serialBufferIndex;
        packetUnion packetConv;

        packet packetBuffer[PACKET_BUFFER_LENGTH];
        uint8_t packetBufferIndex;
};
