# Licence: GPL v.3 http://www.gnu.org/licenses/gpl.html
# XBMC connection to BC127 Bluetooth interface 
# Sparkfun Product: https://www.sparkfun.com/products/11924
# lvgeek: https://github.com/lvgeek

import calendar
import datetime
import sys
import os
import xbmc
import xbmcgui
import xbmcaddon
import socket
import threading
import time

#from BC127 import BC127_io


###################################################################################################
###################################################################################################
# Initialization
###################################################################################################
###################################################################################################
KEY_BUTTON_BACK = 275
KEY_KEYBOARD_ESC = 61467

ACTION_PARENT_DIR = 9
ACTION_PREVIOUS_MENU = 10
ACTION_NAV_BACK = 92

TEXT_ALIGN_LEFT = 0
TEXT_ALIGN_RIGHT = 1
TEXT_ALIGN_CENTER_X = 2
TEXT_ALIGN_CENTER_Y = 4
TEXT_ALIGN_RIGHT_CENTER_Y = 5
TEXT_ALIGN_CENTER_X_CENTER_Y = 6

# Get global paths
addon = xbmcaddon.Addon(id = "script.audio.BT_BC127_AVRCP")
resourcesPath = os.path.join(addon.getAddonInfo('path'),'resources') + '/'
clientScript = os.path.join(addon.getAddonInfo('path'),'resources','jamboree_client.py')
mediaPath = os.path.join(addon.getAddonInfo('path'),'resources','media') + '/'

# Open socket for communication with the gpio-manager UDP server
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
addonW = 1280
addonH = 720

# Buttons Configuration
FREQ_LABEL_X	= 70
FREQ_LABEL_Y	= 220
FREQ_LABEL_W	= 450
FREQ_LABEL_H	= 110
FREQ_LABEL_FONT = 'WeatherTemp'
BUTTON_SEEK_LEFT_X = FREQ_LABEL_X
BUTTON_SEEK_LEFT_Y = FREQ_LABEL_Y + FREQ_LABEL_H
BUTTON_SEEK_LEFT_W = 164
BUTTON_SEEK_LEFT_H = 117
BUTTON_STORE_X = BUTTON_SEEK_LEFT_X + BUTTON_SEEK_LEFT_W
BUTTON_STORE_Y = BUTTON_SEEK_LEFT_Y
BUTTON_STORE_W = 150
BUTTON_STORE_H = 117
BUTTON_SEEK_RIGHT_X = BUTTON_SEEK_LEFT_X + BUTTON_SEEK_LEFT_W + BUTTON_STORE_W
BUTTON_SEEK_RIGHT_Y = BUTTON_SEEK_LEFT_Y
BUTTON_SEEK_RIGHT_W = 164
BUTTON_SEEK_RIGHT_H = 117

RADIO_TEXT_X	= 20
RADIO_TEXT_Y	= 640
RADIO_TEXT_W	= 1280
RADIO_TEXT_H	= 100
RADIO_TEXT_FONT = 'font40_title'

STATION_NAME_W	= 300
STATION_NAME_H	= 100
STATION_NAME_X	= addonW - STATION_NAME_W
STATION_NAME_Y	= 15
STATION_NAME_FONT = 'font40_title'

RSSI_X	= 570
RSSI_Y	= 450
RSSI_W	= 70
RSSI_H	= 10
RSSI_FONT = 'font30'

STATION_LIST_X = 600
STATION_LIST_Y = 220
STATION_LIST_W = 500
STATION_LIST_H = 500
STATION_LIST_FONT = 'font30'

# Presets Configuration
presets_start_x = 149
presets_start_y = 422
presets_offset_x = 160
presets_width = 164
presets_height = 107
presets_font = 'font50_title'
presets_color = '0xFFFFFFFF'
presets_img_left = mediaPath + "left.png"
presets_img_left_focus = mediaPath + "left_focus.png"
presets_img_right = mediaPath + "right.png"
presets_img_right_focus = mediaPath + "right_focus.png"
presets_img_middle = mediaPath + "middle.png"
presets_img_middle_focus = mediaPath + "middle_focus.png"


RADIO_VOL_X = 570
RADIO_VOL_Y = 450
RADIO_VOL_W = 50
RADIO_VOL_H = 1

volume_buttons_x = 1200
volume_buttons_y = 210
volume_label_w = 350
volume_label_x = addonW - volume_label_w
volume_label_y = 80


# Current Frequency label
currentFreq = xbmcgui.ControlLabel(
	FREQ_LABEL_X, FREQ_LABEL_Y,
	FREQ_LABEL_W, FREQ_LABEL_H,
	addon.getLocalizedString(id=30000),
	textColor='0xffffffff',
	font=FREQ_LABEL_FONT,
	alignment=TEXT_ALIGN_RIGHT)

stationsList = xbmcgui.ControlList(
	STATION_LIST_X, 
	STATION_LIST_Y, 
	STATION_LIST_W, 
	STATION_LIST_H, 
	STATION_LIST_FONT,
	buttonTexture=mediaPath + "right.png",
	buttonFocusTexture=mediaPath + 'right_focus.png')
stationsList.setSpace(60)

# Current volume label
currentVolume = xbmcgui.ControlLabel(
	volume_label_x, volume_label_y,
	350, 100,
	'',
	textColor='0xffffffff',
	font='font30',
	alignment=TEXT_ALIGN_RIGHT)

# Radio Text label
radioText = xbmcgui.ControlLabel(
	RADIO_TEXT_X, RADIO_TEXT_Y,
	RADIO_TEXT_W, RADIO_TEXT_H,
	addon.getLocalizedString(id=30000),
	textColor='0xffffffff',
	font=RADIO_TEXT_FONT,
	alignment=TEXT_ALIGN_LEFT)

# Station Name label
stationName = xbmcgui.ControlLabel(
	STATION_NAME_X, STATION_NAME_Y,
	STATION_NAME_W, STATION_NAME_H,
	addon.getLocalizedString(id=30000),
	textColor='0xffffffff',
	font=STATION_NAME_FONT,
	alignment=TEXT_ALIGN_RIGHT)
###################################################################################################
###################################################################################################
# Temperature update thread
###################################################################################################
###################################################################################################
class updateThreadClass(threading.Thread):
	def run(self):
		self.shutdown = False

		while not self.shutdown:
			#currentRssi = int(xbmcgui.Window(10000).getProperty('Radio.RSSI'))
			#Radio_SendCommand(self, "update_rds")

			# Set labels
			currentFreq.setLabel(str(xbmcgui.Window(10000).getProperty('Radio.Frequency')) + "MHz")
			radioText.setLabel(str(xbmcgui.Window(10000).getProperty('Radio.RadioText')))
			stationName.setLabel(str(xbmcgui.Window(10000).getProperty('Radio.StationName')))
			currentVolume.setLabel("volume: " + str(xbmcgui.Window(10000).getProperty('Radio.Volume')))

			# Don't kill the CPU
			time.sleep(0.1)

class PhoneAVRCP(xbmcgui.WindowDialog):
    def __init__(self):
        # Background
        #self.w = self.getWidth()
        #self.h = self.getHeight()
        self.w = addonW
        self.h = addonH
        self.background=xbmcgui.ControlImage(0, 0, self.w, self.h-40, mediaPath + "Carbon-Fiber-9.jpg")
        self.addControl(self.background)

        # top and bottom images
        #self.addControl(xbmcgui.ControlImage(0, 0, self.w, 90, mediaPath + "top_bar.png"))
        #self.addControl(xbmcgui.ControlImage(0, self.h - 90, self.w, 90, mediaPath + "bottom_bar.png"))

        # phone logo
        self.addControl(xbmcgui.ControlImage(self.w - 90, 0, 90, 90, mediaPath + "phoneW.png"))
        self.addControl(xbmcgui.ControlLabel(
            self.w/2 - 150, 25,
            300, 100,
            "Bluetooth Music",
            textColor='0xffffffff',
            font='font30_title',
            alignment=TEXT_ALIGN_CENTER_X))

    
        # Invisible button used to control focus
        self.buttonfocus = xbmcgui.ControlButton(500, 0, 1, 1, "")
        self.addControl(self.buttonfocus)
        self.setFocus(self.buttonfocus)

        # Home button
        self.button_home=xbmcgui.ControlButton(0, 0, 83, 83,
                                                "",
                                                "floor_buttonFO.png",
                                                "floor_button.png",
                                                0,
                                                0)
        self.addControl(self.button_home)
        self.addControl(xbmcgui.ControlImage(0, 0, 83, 83,
                                                mediaPath + "icon_home.png"))

        # Back button
        self.button_back_img=xbmcgui.ControlImage(self.w - 90, self.h - 130, 83, 83,
                                                "icon_back_w.png")
        self.button_back=xbmcgui.ControlButton(self.w - 90, self.h - 130, 83, 83,
                                                "",
                                                "floor_buttonFO.png",
                                                "floor_button.png",
                                                0,
                                                0)
        self.addControl(self.button_back)
        self.addControl(self.button_back_img)

		self.addControl(currentFreq)

		# Left button
		self.button_left=xbmcgui.ControlButton(BUTTON_SEEK_LEFT_X,
												BUTTON_SEEK_LEFT_Y,
												BUTTON_SEEK_LEFT_W,
												BUTTON_SEEK_LEFT_H,
												"",
												mediaPath + "prev_focus.png",
												mediaPath + "prev.png")
		self.addControl(self.button_left)
		self.setFocus(self.button_left)

		# Right button
		self.button_right=xbmcgui.ControlButton(BUTTON_SEEK_RIGHT_X,
												BUTTON_SEEK_RIGHT_Y,
												BUTTON_SEEK_RIGHT_W,
												BUTTON_SEEK_RIGHT_H,
												"",
												mediaPath + "next_focus.png",
												mediaPath + "next.png")
		self.addControl(self.button_right)
		self.setFocus(self.button_right)

		# Add Labels
        self.addControl(messagelabel)
#        self.addControl(CurrentTitle)


        # Start update thread
        self.updateThread = updateThreadClass()
        self.updateThread.start()

        # Store original window ID
        self.prevWindowId = xbmcgui.getCurrentWindowId()
        
        # Go to x11 skin page
        xbmc.executebuiltin("XBMC.ActivateWindow(1112)")
        self.dissablekeys(False)
        
        self.setFocus(self.buttonfocus)

    def onAction(self, action):
        # Escape key
        buttonCode = action.getButtonCode()
        actionID = action.getId()
        if (actionID in (ACTION_PREVIOUS_MENU, ACTION_NAV_BACK, ACTION_PARENT_DIR)):
            strWndFnc = "XBMC.ActivateWindow(10000)"
            xbmc.executebuiltin(strWndFnc)
            # stop the temp thread
            sock.close()
            self.updateThread.shutdown = True
            self.updateThread.join()
            self.close()

    def onControl(self, controlID):
        # Back button
        if controlID == self.button_back:
            strWndFnc = "XBMC.ActivateWindow(%i)" % self.prevWindowId
            xbmc.executebuiltin(strWndFnc)
            # stop the temp thread
            sock.close()
            self.updateThread.shutdown = True
            self.updateThread.join()
            self.close()
        
        # HOME button
        if controlID == self.button_home:
            strWndFnc = "XBMC.ActivateWindow(10000)"
            xbmc.executebuiltin(strWndFnc)
            # stop the temp thread
            sock.close()
            self.updateThread.shutdown = True
            self.updateThread.join()
            self.close()




'''
seek_right
seek_left
tune_xx.x
volume_xx
toggle_mute
get_frequency
'''
def BC127_SendCommand(self, command):
    UDP_IP = "127.0.0.1"
    UDP_PORT = 5555

    # Send request to server
    sock.sendto("BC127_" + command + "\0", (UDP_IP, UDP_PORT))

    # Get reply from server
    #data, addr = sock.recvfrom(1024)

    return #data
    

# Start the Addon
dialog = PhoneAVRCP()
dialog.doModal()
sock.close()
del dialog
del addon
