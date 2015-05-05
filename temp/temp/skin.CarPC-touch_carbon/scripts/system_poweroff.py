import os
import xbmc
import time


xbmc.executebuiltin('XBMC.Quit');
time.sleep(1)
os.system("sudo halt");

#os.system("xdotool keydown Num Plus; xdotool keyup Num Plus");
#os.system("sudo /home/pi/src/carpc-controller/carpc-controller /home/pi/config/gpio_description > /home/pi/log&")
