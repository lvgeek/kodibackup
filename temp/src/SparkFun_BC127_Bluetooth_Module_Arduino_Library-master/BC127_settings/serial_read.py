#! /usr/bin/env python

import serial

ser = serial.Serial('/dev/ttyUSB0')

ser.timeout = 1

while 1:
    print ser.read(1)


