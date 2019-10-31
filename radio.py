import serial
import time
from threading import Thread

class radioPacket():
    def __init__(self, data):
        self.type = data[0]
        self.checksum = data[1]
        self.seqNum = self.interpret16Bit(data[2:4])
        self.payload = data[4:12]

    def interpret24Bit(self, data):
        low = data[0]
        mid = data[1]
        high = data[2]
        return (low) + (mid * (2**8)) + (high * (2**16))

    def interpret16Bit(self, data):
        low = data[0]
        high = data[1]
        return (low) + (high * (2**8))


class setupPacket(radioPacket):
    def __init__(self, data):
        super().__init__(data)
        self.force = self.interpret24Bit(self.payload[0:])
        self.pressure = self.interpret24Bit(self.payload[3:])
        self.continuity = bool(self.payload[6])

    def __str__(self):
        out = "Force: {}, Pressure: {}, Continuity: {}".format(self.force, self.pressure, self.continuity)
        return out


class errorPacket(radioPacket):
    def __init__(self, data):
        super().__init__(data)
        self.storageError = self.payload[0]
        self.adcError = self.payload[1]

    def __str__(self):
        out = "Storage: {}, ADC: {}".format(self.storageError, self.adcError)
        return out


class resultPacket(radioPacket):
    def __init__(self, data):
        super().__init__(data)
        self.time = self.interpret16Bit(self.payload)
        self.force = self.interpret24Bit(self.payload[2:])
        self.pressure = self.interpret24Bit(self.payload[5:])

    def __str__(self):
        out = "#: {}, Time: {}, Force: {}, Pressure: {}".format(self.seqNum, self.time, self.force, self.pressure)
        return out


class RadioManager():
    PACKET_SIZE = 12
    PREAMBLE = [0xAA, 0xBB]
    ESCAPE = 0x11

    PACKET_TYPE_MAP = {
        0: setupPacket,
        1: errorPacket,
        2: resultPacket
    }

    def __init__(self, port):
        self.toSend = []
        self.port = port
        self.serialThread = Thread(target=self._serialThread)

        self.results = {}

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

    def buildPacket(self, packetData):
        packetCons = RadioManager.PACKET_TYPE_MAP[packetData[0]]
        pack = packetCons(packetData)
        if type(pack) is resultPacket:
            self.results[pack.seqNum] = pack
        else:
            print(pack)

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
                            self.buildPacket(packetBuff)
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
for i in range(0, 5):
    rm.sendPacket(128, 0, [0x00, 0x1B, 0, 0, 0, 0, 0, 0])
    time.sleep(0.05)


def animate(i):
    global lastLen
    if len(rm.results) > 10 and len(rm.results) > lastLen:
        lastLen = len(rm.results)
        x = []
        y = []
        recv = list(rm.results.keys())
        recv.sort()
        for p in recv:
            x.append(rm.results[p].time)
            y.append((rm.results[p].force - 110000) * 70 / 390000)
        ax.clear()
        plt.title('{}/1000'.format(len(rm.results)))
        ax.plot(x, y)

import matplotlib.pyplot as plt
import matplotlib.animation as animation

lastLen = 0

fig = plt.figure()
ax = fig.add_subplot(1,1,1)
ani = animation.FuncAnimation(fig, animate, interval=250)
plt.show()
