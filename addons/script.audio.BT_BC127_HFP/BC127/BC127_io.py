#!/usr/bin/env python
import serial
import string
import time
import threading
import xbmc

def BC127_debug(s):
	xbmc.log("[script.BC127] - " + str(s))

class BC127Port:
	 """ BC127Port abstracts all communication with BC127 Jamboree board."""
	 def __init__(self,device,baud=9600):
		 """Initializes port by resetting device. """
		 # These should really be set by the user.
		 databits = 8
		 par      = serial.PARITY_NONE  # parity
		 sb       = 1                   # stop bits
		 to       = 2

		 try:
			 self.port = serial.Serial(device,baud, \
			 parity = par, stopbits = sb, bytesize = databits,timeout = to)
		 except: #serial.serialutil.SerialException:
			 print "PortFailed"
			 raise

		 BC127_debug(self.port.portstr)
		 ready = "ERROR"
		 while ready == "ERROR":
			 ready = self.get_result()[-6:-1]
			 BC127.log("BC127 - " + str(ready))
	 
	 def close(self):
		 self.port.close()
		 BC127_debug("closed port")
		 self.port = None

	 def send_command(self, cmd):
		 """Internal use only: not a public interface"""
		 if self.port:
			 self.port.flushOutput()
			 self.port.flushInput()
			 for c in cmd:
				 self.port.write(c)
			 self.port.write("\r")

	 def interpret_result(self,code):
		 """Internal use only: not a public interface"""
		 # Code will be the string returned from the device.
		 # get the first thing returned, echo should be off

		 code = string.split(code, "\r")
		 code = code[0]
		 
		 #remove whitespace
		 code = string.split(code)
		 code = string.join(code, "")
			

		 if code[:6] == "NODATA": 
			 return "NODATA"

		 code = code[4:]
		 return code
	
	 def get_result(self):
		 """Internal use only: not a public interface"""
		 if self.port:
			 buffer = ""
			 while 1:
				 c = self.port.read(1)
				 if c == '>' and len(buffer) > 0:
					 break
				 else:
					 buffer = buffer + c
			 return buffer.strip()
		 return None

