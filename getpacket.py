import serial, time

def prepPacket(packType, seq, payload):
    pack = [packType, seq] + payload
    pack.append((256 - sum(pack)) % 256)
    return bytearray(pack)

class radioPacket():
    def __init__(self, data):
        self.type = data[0]
        self.checksum = data[1]
        self.seqNum = data[2:4]
        self.payload = data[4:12]

    def interpretADCReading(self, startIndex):
        low = self.payload[startIndex]
        mid = self.payload[startIndex + 1]
        high = self.payload[startIndex + 2]
        return (low) + (mid * (2**8)) + (high * (2**16))


class setupPacket(radioPacket):
    def __init__(self, data):
        super().__init__(data)
        self.force = self.interpretADCReading(0)
        self.pressure = self.interpretADCReading(3)
        self.continuity = bool(self.payload[6])

    def __str__(self):
        out = "Force: {}, Pressure: {}, Continuity: {}".format(self.force, self.pressure, self.continuity)
        return out


class RadioManager():
    PACKET_SIZE = 12
    PREAMBLE = [0xAA, 0xBB]
    ESCAPE = 0x11

    def __init__(self, port):
        self.serialBuffer = []
        self.port = port

    @staticmethod
    def checkPacket(packet):
        checksum = (sum(packet) % 256) == 0
        rightLength = len(packet) == RadioManager.PACKET_SIZE
        return checksum and rightLength

    def run(self):
        escape = False
        packetBuff = []
        inPreamble = False
        inPacket = False
        while True:
            b = int.from_bytes(self.port.read(), 'big')
            if b == RadioManager.PREAMBLE[0] and not escape:
                inPreamble = True
                inPacket = False
            elif b == RadioManager.PREAMBLE[1] and not escape and inPreamble:
                packetBuff = []
                inPacket = True
            elif inPacket and (b != RadioManager.ESCAPE or escape):
                packetBuff.append(b)
                if self.checkPacket(packetBuff):
                    pack = setupPacket(packetBuff)
                    print(pack)
                    inPacket = False
            escape = b == RadioManager.ESCAPE


with serial.Serial('/dev/ttyUSB0', 9600) as ser:
    rm = RadioManager(ser)
    rm.run()
