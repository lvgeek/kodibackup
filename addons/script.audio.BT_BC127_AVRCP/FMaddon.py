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



###################################################################################################
###################################################################################################
# Initialization
###################################################################################################
###################################################################################################
ACTION_PREVIOUS_MENU = 10
ACTION_SELECT_ITEM = 7

TEXT_ALIGN_LEFT = 0
TEXT_ALIGN_RIGHT = 1
TEXT_ALIGN_CENTER_X = 2
TEXT_ALIGN_CENTER_Y = 4
TEXT_ALIGN_RIGHT_CENTER_Y = 5
TEXT_ALIGN_LEFT_CENTER_X_CENTER_Y = 6


# Get global paths
addon = xbmcaddon.Addon(id = "plugin.program.radioFM")
#getADCvalScript = os.path.join(addon.getAddonInfo('path'),'resources','MCP3008-adcread.py')
#motorControlScript = os.path.join(addon.getAddonInfo('path'),'resources','L293D-stepper.py')
resourcesPath = os.path.join(addon.getAddonInfo('path'),'resources') + '/'
clientScript = os.path.join(addon.getAddonInfo('path'),'resources','radio_client.py')
stations = os.path.join(addon.getAddonInfo('path'),'resources','stations')
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

# RSSI indicator
signalStrengthBar = xbmcgui.ControlImage(
	RSSI_X,
	RSSI_Y,
	RSSI_W,
	RSSI_H,
	mediaPath + "bottom_bar.png")
rssi = xbmcgui.ControlLabel(
	RSSI_X, RSSI_Y - 35,
	RSSI_W, RSSI_H,
	'',
	textColor='0xffffffff',
	font=RSSI_FONT,
	alignment=TEXT_ALIGN_CENTER_X)


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

			'''
			radioVolume = int(xbmcgui.Window(10000).getProperty('Radio.Volume'))
			if radioVolume > 0:
				radioVolumeBar.setPosition(RADIO_VOL_X, RADIO_VOL_Y - radioVolume * 10)
				radioVolumeBar.setHeight(radioVolume * 10)
			else:
				radioVolumeBar.setHeight(10)'''
			'''
			currentRssi = int(xbmcgui.Window(10000).getProperty('Radio.RSSI'))
			if currentRssi > 0:
				signalStrengthBar.setPosition(RADIO_VOL_X, RADIO_VOL_Y - currentRssi * 10)
				signalStrengthBar.setHeight(currentRssi * 10)
			else:
				signalStrengthBar.setHeight(10)
			rssi.setLabel(str(currentRssi))
			'''
			# Don't kill the CPU
			time.sleep(0.1)

class RadioFM(xbmcgui.WindowDialog):

	def __init__(self):
		# Background
		#self.w = self.getWidth()
		#self.h = self.getHeight()
		self.w = addonW
		self.h = addonH
		self.background=xbmcgui.ControlImage(0, 0, self.w, self.h, mediaPath + "background.png")
		self.addControl(self.background)

		#win = xbmcgui.Window(xbmcgui.getCurrentWindowId())
		#category = str(win.getProperty('frequency'))

		# top and bottom images
		self.addControl(xbmcgui.ControlImage(0, 0, self.w, 90, mediaPath + "top_bar.png"))
		self.addControl(xbmcgui.ControlImage(0, self.h - 90, self.w, 90, mediaPath + "bottom_bar.png"))

		# radio logo
		#self.addControl(xbmcgui.ControlImage(self.w/2 - 143/2, 80, 143, 57, mediaPath + "logo_radio.png"))
		self.addControl(xbmcgui.ControlLabel(
			90, 25,
			300, 100,
			"Radio FM",
			textColor='0xffffffff',
			font='font30_title',
			alignment=TEXT_ALIGN_LEFT))

		'''
		Pressing channels buttons:
			mode = 0 -> go to channel
			mode = 1 -> memorate curent channel
		'''
		self.mode = 0
		
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
		self.button_back_img=xbmcgui.ControlImage(self.w - 100, self.h - 83, 83, 83,
												"icon_back_w.png")
		self.button_back=xbmcgui.ControlButton(self.w - 100, self.h - 83, 83, 83,
												"",
												"floor_buttonFO.png",
												"floor_button.png",
												0,
												0)
		self.addControl(self.button_back)
		self.addControl(self.button_back_img)

		# text background
		#self.addControl(xbmcgui.ControlImage(BUTTON_SEEK_LEFT_X, 
		#										BUTTON_SEEK_LEFT_Y - BUTTON_SEEK_LEFT_H, 
		#										380 + 10, 
		#										125, 
		#										mediaPath + "text_background.png"))
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

		# Store Station Button
		self.button_store=xbmcgui.ControlButton(BUTTON_STORE_X,
												BUTTON_STORE_Y,
												BUTTON_STORE_W,
												BUTTON_STORE_H,
												"",
												mediaPath + "settings_focus.png",
												mediaPath + "settings.png",
												0,
												0,
												alignment=TEXT_ALIGN_CENTER_X)
		self.addControl(self.button_store)
		self.setFocus(self.button_store)

		# Volume up button
		self.button_volume_up=xbmcgui.ControlButton(volume_buttons_x,
												volume_buttons_y - 70,
												70,70,
												'',
												mediaPath + 'volume-up2.png',
												mediaPath + 'volume-up.png',
												0,
												0,
												alignment=TEXT_ALIGN_CENTER_X)
		self.addControl(self.button_volume_up)
		self.setFocus(self.button_volume_up)

		# Volume down button
		self.button_volume_down=xbmcgui.ControlButton(volume_buttons_x,
												volume_buttons_y,
												70,70,
												'',
												mediaPath + 'volume-down2.png',
												mediaPath + 'volume-down.png',
												0,
												0,
												alignment=TEXT_ALIGN_CENTER_X)
		self.addControl(self.button_volume_down)
		self.setFocus(self.button_volume_down)

		# Volume mute button
		'''
		self.button_volume_mute=xbmcgui.ControlButton(volume_buttons_x,
												volume_buttons_y + 200,
												100,100,
												'',
												mediaPath + 'icons/volume-mute.png',
												mediaPath + 'icons/volume-mute2.png',
												0,
												0,
												alignment=TEXT_ALIGN_CENTER_X)
		self.addControl(self.button_volume_mute)
		self.setFocus(self.button_volume_mute)'''

		self.addControl(currentVolume)

		# Add Labels
		self.addControl(radioText)
		self.addControl(stationName)

		# Stations list
		self.addControl(stationsList)

		# Get stations from the file and show them
		updateStations(self)

		# Start temperature update thread
		self.updateThread = updateThreadClass()
		self.updateThread.start()

		#self.addControl(signalStrengthBar)
		#self.addControl(rssi)

		# Store original window ID
		self.prevWindowId = xbmcgui.getCurrentWindowId()
		
		# Go to x11 skin page
		xbmc.executebuiltin("XBMC.ActivateWindow(1114)")
		
		self.setFocus(self.buttonfocus)
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

		# Seek right Radio channel
		if controlID == self.button_left:
			Radio_SendCommand(self, "seek_down")
			#Radio_SendCommand(self, "tune_down")
			self.setFocus(self.buttonfocus)
			#xbmc.executebuiltin("XBMC.SetProperty(frequency,"+dat+"FM")
			#xbmcgui.Dialog().ok("New Channel found",channel)

		# Seek left Radio channel
		if controlID == self.button_right:
			Radio_SendCommand(self, "seek_up")
			#Radio_SendCommand(self, "tune_up")
			self.setFocus(self.buttonfocus)

		# Store Station
		if controlID == self.button_store:
			currentFrequency = str(xbmcgui.Window(10000).getProperty('Radio.Frequency'))
			currentStationName = str(xbmcgui.Window(10000).getProperty('Radio.StationName'))
			currentFrequencyInList = False
			for i in range(0, len(self.stations_array)):
				currentItem = str(self.stations_array[i])
				#print "%s %s %i" % (currentItem, currentFrequency, currentItem.find(currentFrequency))
				if currentItem.find(currentFrequency) >= 0:
					#xbmcgui.Dialog().ok("%s already in list" % currentFrequency, "Press OK to continue")
					currentFrequencyInList = True
					break
			if currentFrequencyInList == False:
				if currentStationName:
					currentFrequency = currentFrequency + " - " + currentStationName
				stationsList.addItem(currentFrequency)
				self.stations_array.append(currentFrequency)
				StationsListUpdatePosition(self)
				writeStations(self)
			self.setFocus(self.buttonfocus)

		# Volume Up
		if controlID == self.button_volume_up:
			Radio_SendCommand(self, "volume_plus")
			'''
			tempVol = int(dat) * 6
			if tempVol >= 90:
				tempVol = 100
			xbmc.executebuiltin("XBMC.SetVolume(%d,1)" % (int(tempVol)))
			'''
			self.setFocus(self.buttonfocus)
		
		# Volume Down
		if controlID == self.button_volume_down:
			Radio_SendCommand(self, "volume_minus")
			'''
			tempVol = int(dat) * 6
			if tempVol >= 90:
				tempVol = 100
			xbmc.executebuiltin("XBMC.SetVolume(%d,1)" % (int(tempVol)))
			'''
			self.setFocus(self.buttonfocus)
		
		# Volume Mute
		'''if controlID == self.button_volume_mute:
			Radio_SendCommand(self, "toggle_mute")
			self.setFocus(self.buttonfocus)'''

		if controlID == stationsList:
			item = stationsList.getSelectedItem()
			requestedFrequency = item.getLabel().split(' ')
			Radio_SendCommand(self, "tune_" + requestedFrequency[0])
			if str(xbmcgui.Window(10000).getProperty('Radio.Active')) == "false":
				xbmc.Player().stop()
				CarpcController_SendCommand("system_mode_toggle")
			self.setFocus(self.buttonfocus)

def CarpcController_SendCommand(command):
	UDP_IP = "127.0.0.1"
	UDP_PORT = 5005

	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

	# Send request to server
	sock.sendto(command + "\0", (UDP_IP, UDP_PORT))

	sock.close()

def StationsListUpdatePosition(self):
	stationsList.setPosition(STATION_LIST_X, STATION_LIST_Y - (10 / 2 * len(self.stations_array)))

'''
Read stations from the stations file and populate stations_array array
'''
def updateStations(self):
	# update channel list from the stations file
	fd = open(stations,"r")
	self.stations_array = []
	for line in fd:
		self.stations_array.append(line.strip("\n"))
	fd.close()

	StationsListUpdatePosition(self)

	for i in range(0, len(self.stations_array)):
		stationsList.addItem(str(self.stations_array[i]))

'''
Update stations_array at position 'index' with frequency 'freq' and
write stations_array to the stations file.
'''
def writeStations(self):
	fd = open(stations,'w')
	for line in self.stations_array:
		fd.write(line+'\n')
	fd.close()

'''
seek_right
seek_left
tune_xx.x
volume_xx
toggle_mute
get_frequency
'''
def Radio_SendCommand(self, command):
	UDP_IP = "127.0.0.1"
	UDP_PORT = 5005

	# Send request to server
	sock.sendto("radio_" + command + "\0", (UDP_IP, UDP_PORT))

	


# Start the Addon
dialog = RadioFM()
dialog.doModal()
sock.close()
del dialog
del addon
