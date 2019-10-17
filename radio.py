import serial
import time
from threading import Thread

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
        self.toSend = []
        self.port = port
        self.serialThread = Thread(target=self._serialThread)

    @staticmethod
    def checkPacket(packet):
        checksum = (sum(packet) % 256) == 0
        rightLength = len(packet) == RadioManager.PACKET_SIZE
        return checksum and rightLength

    def sendPacket(self, packetType, seqNum, payload):
        seqNumLow = seqNum & 0xFF
        seqNumHigh = (seqNum >> 8) & 0xFF
        pack = [packetType, 0, seqNumHigh, seqNumLow] + payload
        pack[1] = (256 - sum(pack)) % 256
        self.toSend.append(bytearray(RadioManager.PREAMBLE + pack))

    def _serialThread(self):
        with serial.Serial(self.port, 9600) as serport:
            escape = False
            packetBuff = []
            inPreamble = False
            inPacket = False
            while True:
                while serport.in_waiting > 0 and len(self.toSend) == 0:
                    b = int.from_bytes(serport.read(), 'big')
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

                if len(self.toSend) > 0:
                    packet = self.toSend.pop(0)
                    serport.write(packet)

    def run(self):
        self.serialThread.start()


rm = RadioManager('/dev/ttyUSB0')
rm.run()
time.sleep(5)
rm.sendPacket(128, 0, [0, 1, 2, 3, 4, 5, 6, 7])
