import xbmc
import time
import serial
 
class XBMCPlayer(xbmc.Player):
    def __init__(self, *args):
        pass
 
    def onPlayBackStarted(self):
        xbmc.log("Playback started - LED ON")
#        ledOn()
 
    def onPlayBackPaused(self):
        xbmc.log("Playback paused - LED OFF")
#        ledOff()
 
    def onPlayBackResumed(self):
        xbmc.log("Playback resumed - LED ON")
#        ledOn()
 
    def onPlayBackEnded(self):
        xbmc.log("Playback ended - LED OFF")
#        ledOff()
 
    def onPlayBackStopped(self):
        xbmc.log("Playback stopped - LED OFF")
#        ledOff()
 
player = XBMCPlayer()

ser = serial.Serial('/dev/ttyUSB0')
ser.timeout = 1
 
monitor = xbmc.Monitor()
 
while True:
    if monitor.waitForAbort(1): # Sleep/wait for abort for 1 second.
        break # Abort was requested while waiting. Exit the while loop.
    cmd = ser.readline()
    if len(cmd)>1:
        cmd = cmd.strip('\r\n')
        xbmc.log("BC127 " + cmd)
        if cmd == "AVRCP_PLAY":
            xbmc.executebuiltin('ActivateWindow(10500)')
        




