#!/usr/bin/python
"""
    python test_read_2.py
"""

import serial


ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=30)
while 1:
    answer = ser.readline()
    print answer
device_port.close()
