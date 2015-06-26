#! /usr/bin/env python

from time import sleep
import serial

ser = serial.Serial('/dev/ttyUSB0')
ser.timeout = 1

# Open the file for reading.
with open('jamboree-config.txt', 'r') as infile:
#with open('jbs.txt', 'r') as infile:

    data = infile.read()  # Read the contents of the file into memory.

# Return a list of the lines, breaking at line boundaries.
my_list = data.splitlines()
for line in my_list:
    sout ="SET " + line + "\r"
    print sout
    ser.write(sout)
    sleep(0.125)
    bytesToRead = ser.inWaiting()
    cmd = ser.read(bytesToRead)
    if len(cmd)>1:
        print cmd.strip('\r\n')

#
ser.write("WRITE\r")
sleep(0.125)
bytesToRead = ser.inWaiting()
cmd = ser.read(bytesToRead)
if len(cmd)>1:
    print cmd.strip('\r\n')

sleep(1)
ser.write("STATUS\r")
try:
    while 1:
        sleep(0.1)
        cmd = ser.readline()
        if len(cmd)>1:
            print cmd.strip('\r\n')
except KeyboardInterrupt:
    pass

ser.close()

