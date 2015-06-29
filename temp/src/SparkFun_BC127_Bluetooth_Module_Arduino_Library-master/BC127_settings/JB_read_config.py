#! /usr/bin/env python

import serial

ser = serial.Serial('/dev/ttyUSB0')

ser.timeout = 1

ser.write("CONFIG\r")

try:
    while 1:
        cmd = ser.readline()
        if len(cmd)>1:
            print cmd.strip('\r\n')
except KeyboardInterrupt:
    pass

ser.close()



