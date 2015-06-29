#!/usr/bin/python
"""
    python test_read_2.py
"""

import serial


ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
while 1:
    answer = ser.readline()
    print answer
device_port.close()
