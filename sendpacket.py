import serial, time

message = [
    2,
    0,
    0, 0, 0, 1, 0, 4, 0, 0,
    256 - (2+1+4)
]

def prepPacket(packType, seq, payload):
    pack = [packType, seq] + payload
    pack.append((256 - sum(pack)) % 256)
    return bytearray(pack)

eM1 = prepPacket(2, 0, [128, 210, 128, 1, 4, 4, 225, 10])
eM2 = prepPacket(2, 1, [128, 210, 128, 1, 4, 4, 225, 10])
eM3 = prepPacket(2, 2, [128, 210, 128, 1, 4, 4, 225, 10])

print(eM1)
print(eM2)
print(eM3)

firePacket = prepPacket(128, 0, [0, 1, 2, 3, 4, 5, 6, 7])

with serial.Serial('COM20', 9600) as ser:
    ser.write(firePacket)
