#include <arduino.h>

#define PACKET_PAYLOAD_LENGTH 8

#define PACKET_BUFFER_LENGTH 8

#define RADIO_PREABLE_0 0xAA
#define RADIO_PREABLE_1 0xBB
#define RADIO_ESCAPE 0x11

typedef struct radioPacket {
    uint8_t type;
    uint8_t checksum;
    uint16_t seqNum;
    uint8_t payload[PACKET_PAYLOAD_LENGTH];
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
        void resetBuffers();
    
        bool validatePacket(packet* testPack);

        uint8_t available();
        packet readPacket();
        void resetPacketBuffer();

        void sendPacket(packet toSend);

    private:
        uint8_t serialBufferIndex;
        packetUnion packetConv;

        packet packetBuffer[PACKET_BUFFER_LENGTH];
        uint8_t packetBufferIndex;

        // Packet parser state
        bool escape = false;
        bool inPreamble = false;
        bool inPacket = false;
};
