import serial, time

message = [
    2,
    0,
    0, 0, 0, 1, 0, 4, 0, 0,
    256 - (2+1+4)
]

PACKET_SIZE = 11

buff = []

def prepPacket(packType, seq, payload):
    pack = [packType, seq] + payload
    pack.append((256 - sum(pack)) % 256)
    return bytearray(pack)

def checkPacket(packet):
    return (sum(packet) % 256) == 0 and len(packet) == PACKET_SIZE

def interpretADCReading(payload, startIndex):
    low = payload[startIndex]
    mid = payload[startIndex + 1]
    high = payload[startIndex + 2]
    return (low) + (mid * (2**8)) + (high * (2**16))

class radioPacket():
    def __init__(self, data):
        if checkPacket(data):
            self.type = data[0]
            self.seqNum = data[1]
            self.payload = data[2:10]
            self.checksum = data[10]
            if self.type == 0:
                self.force = interpretADCReading(self.payload, 0)
                self.pressure = interpretADCReading(self.payload, 3)
        else:
            raise ValueError('Invalid packet data')


class setupPacket(radioPacket):
    def __init__(self, data):
        super().__init__(data)


class RadioManager():
    def __init__(self, port):
        self.serialBuffer = []
        self.port = port

    def update(self):
        if self.port.in_waiting > 0:
            data = self.port.read(self.port.in_waiting)
            data = [byte for byte in data]
            self.serialBuffer += data
            if len(self.serialBuffer) >= PACKET_SIZE:
                for i in range(0, len(self.serialBuffer) - PACKET_SIZE):
                    testPack = self.serialBuffer[i:i + 11]
                    if checkPacket(testPack):
                        self.serialBuffer = self.serialBuffer[i + 11:]
                        pack = radioPacket(testPack)
                        print("Force: " + str(pack.force) + ", Pressure: " + str(pack.pressure))
                

with serial.Serial('COM20', 9600) as ser:
    rm = RadioManager(ser)
    while True:
        rm.update()
