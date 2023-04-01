import numpy as np
import serial.tools.list_ports
import struct

"""
    This script reads mfccs from the esp32 and saves them to a file
    Serial dump needs to be enabled on the esp32
    The esp32 needs to be connected to the computer via usb
    These can be used to train a neural network
    """

#Read single mfsc from serial
def read_mfsc(ser):
    buffer = [0, 0, 0, 0, 0, 0]
    x = 0
    while True:
        buffer.append(ser.read())
        buffer.pop(0)
        if buffer[-1] == b'\xD2' and buffer[-2] == b'\x00' and buffer[-3] == b'\x7E' and buffer[-4] == b'\x7E' and \
                buffer[-5] == b'\x00' and buffer[-6] == b'\xD2':
            print("found token")
            while True:
                buffer.append(ser.read())
                x += 1
                if x > 65100:
                    print("timeout")
                    return None
                if buffer[-1] == b'\x7E' and buffer[-2] == b'\x00' and buffer[-3] == b'\x00' and buffer[
                    -4] == b'\x7E' and buffer[-5] == b'\x00' and buffer[-6] == b'\x7E':
                    print("found token")
                    print(len(buffer[6:-6]))
                    break
            break

    buffer = buffer[6:-6]
    try:
        flat_floats = []
        for x in range(0, len(buffer) - 1, 4):
            float = struct.unpack("<f", b"".join(buffer[x:x + 4]))[0]
            flat_floats.append(float)
    except:
        print("error")
        return None

    array = np.array(flat_floats).reshape(625, 26)

    return array

#Read single mfsc from serial
ports = list(serial.tools.list_ports.comports())
for i, p in enumerate(ports):
    print(i, p)
# choose a port
port = ports[int(input("choose a port: "))]
classes = ["glassbreak", "dog_bark", "baby_cry", "people_talking"]

# open serial connection
ser = serial.Serial(port.device, 115200, timeout=1)
ser.flush()

# print classes
for i, c in enumerate(classes):
    print(i, c)

# Ask user input for class
class_index = int(input("choose a class: "))

# Ask user input for number of mfccs to read
i = int(input("number of mfsc to read: "))
input("press enter to start")

mfscs = np.array([])
x = 0
while x < i:
    c = read_mfsc(ser)
    ser.flush()
    if c is not None:
        mfscs = np.append(mfscs, c)
        print(x)
        x += 1

ser.close()
mfscs = mfscs.reshape(i, 26, 625)
import random

# save mfscs to a file folder outputs with random number
np.savez("output/" + classes[class_index] + "_" + str(random.randint(0, 1000000)), mfccs=mfscs, class_index=class_index)
