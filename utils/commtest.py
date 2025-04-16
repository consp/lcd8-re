import serial
import fcntl
import sys
from binascii import hexlify, unhexlify
import struct
import random
from time import sleep
from crc import Calculator, Crc8
import os
import subprocess

cal = Calculator(Crc8.CCITT)
print(hex(cal.checksum(b"\xaa" * 8)))
DEBUG = 1

# if len(sys.argv) < 2:
#     print("Serial port required as argument.")
#     exit(1)

if len(sys.argv) > 1 and sys.argv[1].startswith("/"):
    port = sys.argv[1]
    baudrate = 57600
    print("Baudrate: %d" % baudrate)
    ser = serial.Serial(port, baudrate, bytesize=8, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=0.1)
else:
    print("Writing to stdin/out")
    ser = None
    # fin = open("/proc/%d/fd/0" % (pid), "wb")
    # fout = open("/proc/%d/fd/1" % (pid), "rb")
    # os.set_blocking(fout.fileno(), False)
    # os.set_blocking(fin.fileno(), False)
    subp = subprocess.Popen("./build/LCD8H-firmware", stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    fd = subp.stdout.fileno()
    fl = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)
    # fd = subp.stdin.fileno()
    # fl = fcntl.fcntl(fd, fcntl.F_GETFL)
    # fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)


voltage = 24000
current = 10000
speed = 25000000 # mm/h
wheel_circ = 2230
cnt = 0

def sendmsg(tag, length, value):
    crc = cal.checksum(bytes([tag, length]) + value)
    blob = bytes([tag, length, crc]) + value
    if ser:
        ser.write(blob)
    else:
        try:
            # subp.communicate(blob, timeout=0.001) 
            subp.stdin.write(blob)
            subp.stdout.flush()
            subp.stdin.flush()
        except subprocess.TimeoutExpired:
            pass
        except:
            raise

while True:
    # blob = b"\x10" + struct.pack(">I", voltage)
    # ser.write(blob)
     
    # blob = b"\x11" + struct.pack(">I", current)
    # ser.write(blob)
    cnt = cnt + 1
    if cnt > 5:
        spd = round(1000 / (speed / 3600 / wheel_circ))
        sendmsg(0x12, 3, struct.pack(">H", spd if spd < 65536 else 65535) + b"\x00")
        speed = speed + 100000 
        cnt = 0
        if (speed > 30000000):
            speed = 1

    if cnt % 2:
        sendmsg(0x11, 4, struct.pack(">i", current))
        current = current + 100 
        if current > 10000:
            current = -10000
        sendmsg(0x10, 4, struct.pack(">i", 28100))
    if cnt == 4:
        sendmsg(0x13, 4, struct.pack(">h", 220) + struct.pack(">h", 150)) 

    if ser:
        data = ser.read(1024)
    else:
        subp.stdout.flush()
        data = subp.stdout.read()
        if data is None:
            data = b""
    if data.startswith(b"\x81"):
        print("Display settings\t\tMax speed: %d km/h, max current: %d mA, wheel circumfence: %d, assist levels: %d, checksum: %s" % (
                data[5],
                struct.unpack(">H", data[3:5])[0],
                struct.unpack(">H", data[6:8])[0],
                data[8],
                data[2] == cal.checksum(data[0:2] + data[3:9])
            ))
    elif data.startswith(b"\x80"):
        print("Display status\t\tAssist: %d, Lights %d, checksum: %s" % (
                data[3],
                data[4],
                data[2] == cal.checksum(data[0:2] + data[3:5])
            ))
    elif len(data) > 0:
        if b'\n' in data:
            try:
                print("Unknown data: %s" % (data.decode()))
            except:
                print("Unknown data: " + hexlify(data).decode())
        else:
            print("Unknown data: " + hexlify(data).decode())
    sleep(0.01)
    # print(1)
