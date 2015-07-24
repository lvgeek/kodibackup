# obd2-xbmc - OBD2 Data Display for XBMC
# Copyright (C) 2014  DAFreeman dave@lvengineering.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

# import the XBMC libraries so we can use the controls and functions of XBMC
import sys
import os
import xbmc
import xbmcgui
import xbmcaddon 

ACTION_PREVIOUS_MENU = 10
ACTION_SELECT_ITEM = 7
ACTION_CANCEL_DIALOG = (9,10,51,92,110)

TEXT_ALIGN_LEFT = 0
TEXT_ALIGN_RIGHT = 1
TEXT_ALIGN_CENTER_X = 2
TEXT_ALIGN_CENTER_Y = 4
TEXT_ALIGN_RIGHT_CENTER_Y = 5
TEXT_ALIGN_LEFT_CENTER_X_CENTER_Y = 6

# name and create our window 
class OBD2XBMC(xbmcgui.WindowXMLDialog): 
	def __init__( self, *args, **kwargs ):
		xbmcgui.WindowXMLDialog.__init__( self )
		pass

	def onInit( self ):
		self._show_credits()

	def _show_credits( self ):
		try:
			xbmcgui.lock()
			'''
			#team credits
			self.getControl( 20 ).setLabel( __scriptname__ )
			self.getControl( 30 ).setLabel( "%s: %s-%s" % ( _( 1006 ), __version__, __svn_revision__, ) )
			self.getControl( 40 ).addLabel( __svn_url__ )
			self.getControl( 901 ).setLabel( _( 901 ) )
			self.getControl( 101 ).reset()
			list_item = xbmcgui.ListItem( sys.modules[ "__main__" ].__credits_l1__, sys.modules[ "__main__" ].__credits_r1__ )
			self.getControl( 101 ).addItem( list_item )
			list_item = xbmcgui.ListItem( sys.modules[ "__main__" ].__credits_l2__, sys.modules[ "__main__" ].__credits_r2__ )
			self.getControl( 101 ).addItem( list_item )
			list_item = xbmcgui.ListItem( sys.modules[ "__main__" ].__credits_l3__, sys.modules[ "__main__" ].__credits_r3__ )
			self.getControl( 101 ).addItem( list_item )
			# Additional credits
			self.getControl( 902 ).setLabel( _( 902 ) )
			self.getControl( 102 ).reset()
			list_item = xbmcgui.ListItem( sys.modules[ "__main__" ].__add_credits_l1__, sys.modules[ "__main__" ].__add_credits_r1__ )
			self.getControl( 102 ).addItem( list_item )
			list_item = xbmcgui.ListItem( sys.modules[ "__main__" ].__add_credits_l2__, sys.modules[ "__main__" ].__add_credits_r2__ )
			self.getControl( 102 ).addItem( list_item )
			list_item = xbmcgui.ListItem( sys.modules[ "__main__" ].__add_credits_l3__, sys.modules[ "__main__" ].__add_credits_r3__ )
			self.getControl( 102 ).addItem( list_item )
			# Skin credits
			self.getControl( 903 ).setLabel( _( 903 ) )
			'''
		except:
			pass
		xbmcgui.unlock()

	def _close_dialog( self ):
		self.close()
  
	def onClick( self, controlId ):
	        pass
  
	def onFocus( self, controlId ):
	        pass
  
#	def onAction( self, action ):
#	        if ( action in ACTION_CANCEL_DIALOG ):
#	            self._close_dialog()


	def __init__(self.script-obd2-xbmc.xml, strFallbackPath, strDefaultName, forceFallback):
		self.retval=0
		self.mediaPath=os.path.join(addon.getAddonInfo('path'),'resources','media') + '/'

		# Background and labels
		self.w = 1280
		self.h = 720
		self.background=xbmcgui.ControlImage(0,0,self.w,self.h,self.mediaPath + 'background_.jpg', colorDiffuse='0xff777777')
		self.addControl(self.background)
		self.title=xbmcgui.ControlLabel(self.w/2 - 400, 20, 800, 100, addon.getLocalizedString(id=30000),textColor='0xffffffff',font='font50_title',alignment=2)
		self.addControl(self.title)
		self.speed=xbmcgui.ControlLabel(3, 100, 120, 50, addon.getLocalizedString(id=30001),textColor='0xffffffff',font='font30_title',	alignment=1)
		self.addControl(self.speed)
		self.RPM=xbmcgui.ControlLabel(3, 150, 120, 50, addon.getLocalizedString(id=30002),textColor='0xffffffff',font='font30_title', alignment=1)
		self.addControl(self.RPM)
		self.Coolant=xbmcgui.ControlLabel(3, 200, 120, 50, addon.getLocalizedString(id=30003),textColor='0xffffffff',font='font30_title', alignment=1)
		self.addControl(self.Coolant)
		self.Vacuum=xbmcgui.ControlLabel(3, 250, 120, 50, addon.getLocalizedString(id=30004),textColor='0xffffffff',font='font30_title', alignment=1)
		self.addControl(self.Vacuum)
		self.Timing=xbmcgui.ControlLabel(3, 300, 120, 50, addon.getLocalizedString(id=30005),textColor='0xffffffff',font='font30_title', alignment=1)
		self.addControl(self.Timing)
		self.Power=xbmcgui.ControlLabel(3, 350, 120, 50, addon.getLocalizedString(id=30006),textColor='0xffffffff',font='font30_title',	alignment=1)
		self.addControl(self.Power)
		self.Torque=xbmcgui.ControlLabel(3, 400, 120, 50, addon.getLocalizedString(id=30007),textColor='0xffffffff',font='font30_title', alignment=1)
		self.addControl(self.Torque)
		self.MPG=xbmcgui.ControlLabel(3, 450, 120, 50, addon.getLocalizedString(id=30008),textColor='0xffffffff',font='font30_title', alignment=1)
		self.addControl(self.MPG)

		# Data labels
		self.dspeed=xbmcgui.ControlLabel(130, 100, 120, 50, '0',textColor='0xffffffff',font='font30_title', alignment=2)
		self.addControl(self.dspeed)
		self.dRPM=xbmcgui.ControlLabel(130, 150, 120, 50, '0',textColor='0xffffffff',font='font30_title', alignment=2)
		self.addControl(self.dRPM)
		self.dCoolant=xbmcgui.ControlLabel(130, 200, 120, 50, '0',textColor='0xffffffff',font='font30_title', alignment=2)
		self.addControl(self.dCoolant)
		self.dVacuum=xbmcgui.ControlLabel(130, 250, 120, 50, '0',textColor='0xffffffff',font='font30_title', alignment=2)
		self.addControl(self.dVacuum)
		self.dTiming=xbmcgui.ControlLabel(130, 300, 120, 50, '0',textColor='0xffffffff',font='font30_title', alignment=2)
		self.addControl(self.dTiming)
		self.dPower=xbmcgui.ControlLabel(130, 350, 120, 50, '0',textColor='0xffffffff',font='font30_title',	alignment=2)
		self.addControl(self.dPower)
		self.dTorque=xbmcgui.ControlLabel(130, 400, 120, 50, '0',textColor='0xffffffff',font='font30_title',	alignment=2)
		self.addControl(self.dTorque)
		self.dMPG=xbmcgui.ControlLabel(130, 450, 120, 50, '0',textColor='0xffffffff',font='font30_title',	alignment=2)
		self.addControl(self.dMPG)

		# Data progress
		self.pspeed=xbmcgui.ControlProgress(250, 100, 400, 25)
		self.addControl(self.pspeed)
		self.pspeed.setPercent(99)


		# Back button
		self.button_back_img=xbmcgui.ControlImage(self.w - 90, self.h - 100, 80, 80, self.mediaPath + 'icon_back.png',colorDiffuse='0xffffffff')
		self.button_back=xbmcgui.ControlButton(self.w - 90, self.h - 100, 80, 80,"",self.mediaPath + 'iconback-focus.png',self.mediaPath + 'iconback.png',0,0)
		self.addControl(self.button_back)
		self.addControl(self.button_back_img)

		# Store original window ID
		self.prevWindowId = xbmcgui.getCurrentWindowId()
		
		
	def onControl(self, controlID):
		# Back button
		if controlID == self.button_back:
			self.retval=0
			self.close()







addon = xbmcaddon.Addon(id='script.obd2-xbmc')
finished=0

while finished == 0:
    dialog=OBD2XBMC()
#	dialog=OBD2XBMC("script-obd2-main.xml", xbmcaddon.Addon().getAddonInfo('path'))
	dialog.doModal()
	if dialog.retval == 0:
		finished = 1
	del dialog
del addon

