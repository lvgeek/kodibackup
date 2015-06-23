#! /usr/bin/env python
"""\
Scan for serial ports. Linux specific variant that also includes USB/Serial
adapters.

Part of pySerial (http://pyserial.sf.net)
(C) 2009 <cliechti@gmx.net>
"""

import serial
import glob



def scan():
    """scan for available ports. return a list of device names.

    pvergain@houx:~/PDEV1V160_CodesRousseau/Soft/PC/test_boost/serialport/pyserial$ python scanlinux.py
    Found ports:
    /dev/ttyS0
    /dev/ttyS3
    /dev/ttyS2
    /dev/ttyS1
    /dev/ttyACM0
    /dev/serial/by-id/usb-id3_semiconductors_MEABOARD_00000000-if00
    """
    return glob.glob('/dev/ttyS*') + glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*') + glob.glob('/dev/serial/by-id/*')

if __name__=='__main__':
    print "Found ports:"
    for name in scan():
        print name
