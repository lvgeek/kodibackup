#! /usr/bin/env python

import serial

ser = serial.Serial('/dev/ttyUSB0')

ser.timeout = 1

while 1:
    cmd = ser.readline()
    if len(cmd)>1:
        print cmd



