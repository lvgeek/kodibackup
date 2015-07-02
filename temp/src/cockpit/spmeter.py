# -*- coding: utf-8 -*-
import wx
import SpeedMeter as SM
from math import pi, sqrt


# This Is For Latin/Greek Symbols I Used In The Demo Only
wx.SetDefaultPyEncoding('iso8859-1')
# Beginning Of SPEEDMETER Demo wxPython Code

class SpeedMeterDemo(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, -1, "",
                         wx.DefaultPosition,
                         size=(400,400),
                         style=wx.DEFAULT_FRAME_STYLE |
                         wx.NO_FULL_REPAINT_ON_RESIZE)     
        
        #self.SetMenuBar(self.CreateMenuBar())
        
        panel = wx.Panel(self, -1)
        sizer = wx.FlexGridSizer(2, 3, 2, 5)

        # 6 Panels To Hold The SpeedMeters ;-)
        
        panel1 = wx.Panel(panel, -1)
        panel2 = wx.Panel(panel, -1)
        panel3 = wx.Panel(panel, -1)
        panel4 = wx.Panel(panel, -1)
        panel5 = wx.Panel(panel, -1)
        panel6 = wx.Panel(panel, -1)
              
        # First SpeedMeter: We Use The Following Styles:        
        self.SpeedWindow1 = SM.SpeedMeter(panel1,
                                          extrastyle=SM.SM_DRAW_HAND |
                                          #SM.SM_DRAW_SECTORS |
                                          SM.SM_DRAW_SECONDARY_TICKS |
                                          SM.SM_DRAW_MIDDLE_TEXT |
                                #SM.SM_DRAW_PARTIAL_FILLER |
                                          SM.SM_ROTATE_TEXT
                                
                                          )
                                
        self.SpeedWindow1.SetAngleRange(-pi/6, 7*pi/6)
        intervals = range(0, 201, 20)
        self.SpeedWindow1.SetIntervals(intervals)
        colours = [wx.BLACK]*10
        self.SpeedWindow1.SetIntervalColours(colours)
        ticks = [str(interval) for interval in intervals]
        self.SpeedWindow1.SetTicks(ticks)
        self.SpeedWindow1.SetTicksColour(wx.WHITE)
        self.SpeedWindow1.SetTicksFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))      
        self.SpeedWindow1.SetNumberOfSecondaryTicks(4)
        self.SpeedWindow1.SetHandColour(wx.Colour(255, 50, 0))      
        self.SpeedWindow1.SetSpeedBackground(wx.BLACK)        
        self.SpeedWindow1.SetHandColour(wx.WHITE)
        self.SpeedWindow1.DrawExternalArc(False)
        self.SpeedWindow1.SetMiddleText("KPH")
        self.SpeedWindow1.SetMiddleTextColour(wx.WHITE)
        self.SpeedWindow1.SetMiddleTextFont(wx.Font(9, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.SpeedWindow1.SetSpeedValue(5.6)
      
       
      # Second SpeedMeter: We Use The Following Styles:
        self.SpeedWindow2 = SM.SpeedMeter(panel5,
                                          extrastyle=SM.SM_DRAW_HAND |
                                          SM.SM_DRAW_SECTORS |
                                SM.SM_DRAW_MIDDLE_TEXT |
                                          SM.SM_DRAW_MIDDLE_ICON,
                                          mousestyle=SM.SM_MOUSE_TRACK
                                          )

        self.SpeedWindow2.SetAngleRange(-pi/6, 7*pi/6)
        intervals = range(0, 81, 10)
        self.SpeedWindow2.SetIntervals(intervals)
        colours = [wx.BLUE]*2
        colours.extend([wx.BLACK]*5)
        colours.append(wx.RED)

        self.SpeedWindow2.SetIntervalColours(colours)
        ticks = [str(interval) + "°" for interval in intervals]
        self.SpeedWindow2.SetTicks(ticks)
        self.SpeedWindow2.SetTicksColour(wx.WHITE)
        self.SpeedWindow2.SetTicksFont(wx.Font(10, wx.TELETYPE, wx.NORMAL, wx.BOLD))
        self.SpeedWindow2.SetMiddleText("INTAKE °C ")
        self.SpeedWindow2.SetMiddleTextColour(wx.WHITE)
        self.SpeedWindow2.SetMiddleTextFont(wx.Font(9, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.SpeedWindow2.SetHandColour(wx.WHITE)
      # We Set The Background Colour Of The SpeedMeter OutSide The Control
        self.SpeedWindow2.SetSpeedBackground(wx.BLACK)
        self.SpeedWindow2.DrawExternalArc(False)
        self.SpeedWindow2.SetHandColour(wx.WHITE)
        # Define The Icon We Want
        icon = wx.Icon("temp.ico", wx.BITMAP_TYPE_ICO)
        icon.SetWidth(14)
        icon.SetHeight(6)
        # Draw The Icon In The Center Of SpeedMeter        
        self.SpeedWindow2.SetMiddleIcon(icon)        
        # Quite An High Fever!!!        
        self.SpeedWindow2.SetSpeedValue(80)

        # Third SpeedMeter: We Use The Following Styles:
          
        self.SpeedWindow3 = SM.SpeedMeter(panel4,
                                          extrastyle=SM.SM_DRAW_HAND |
                                          SM.SM_DRAW_SECTORS |
                                          SM.SM_DRAW_MIDDLE_TEXT |
                                          SM.SM_DRAW_MIDDLE_ICON,
                                          mousestyle=SM.SM_MOUSE_TRACK
                                          )

        # We Want To Simulate Some Kind Of Thermometer (In Celsius Degrees!!!)
        self.SpeedWindow3.SetAngleRange(-pi/6, 7*pi/6)

        intervals = range(20, 121, 10)
        self.SpeedWindow3.SetIntervals(intervals)
        colours = [wx.BLUE]*2
        colours.extend([wx.BLACK]*7)
        colours.append(wx.RED)
        self.SpeedWindow3.SetIntervalColours(colours)
        ticks = [str(interval) + "°" for interval in intervals]
        self.SpeedWindow3.SetTicks(ticks)
        self.SpeedWindow3.SetTicksColour(wx.WHITE)
        self.SpeedWindow3.SetTicksFont(wx.Font(10, wx.TELETYPE, wx.NORMAL, wx.BOLD))        
        self.SpeedWindow3.SetHandColour(wx.WHITE)
        self.SpeedWindow3.SetSpeedBackground(wx.BLACK)        
        self.SpeedWindow3.SetMiddleText("BLOCK °C")
        self.SpeedWindow3.SetMiddleTextColour(wx.WHITE)
        self.SpeedWindow3.SetMiddleTextFont(wx.Font(9, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.SpeedWindow3.DrawExternalArc(False)
        self.SpeedWindow3.SetHandColour(wx.WHITE)      
        icon = wx.Icon("temp.ico", wx.BITMAP_TYPE_ICO)
        icon.SetWidth(14)
        icon.SetHeight(8) 
        self.SpeedWindow3.SetMiddleIcon(icon)           
        self.SpeedWindow3.SetSpeedValue(40)
                
        # Fourth SpeedMeter: We Use The Following Styles:
        # NOTE: We Use The Mouse Style mousestyle=SM_MOUSE_TRACK. In This Way, Mouse
        
        self.SpeedWindow4 = SM.SpeedMeter(panel3,
                                          extrastyle=SM.SM_DRAW_HAND |
                                          SM.SM_DRAW_SECTORS |
                                SM.SM_DRAW_MIDDLE_TEXT |
                                          SM.SM_DRAW_MIDDLE_ICON,
                                          mousestyle=SM.SM_MOUSE_TRACK
                                          )
        self.SpeedWindow4.SetAngleRange(-pi/6, 7*pi/6)
        intervals = range(-5, 46, 5)
        self.SpeedWindow4.SetIntervals(intervals)
        colours = [wx.BLUE]*2
        colours.extend([wx.BLACK]*7)
        colours.append(wx.RED)
        self.SpeedWindow4.SetIntervalColours(colours)
        ticks = [str(interval) + "°" for interval in intervals]
        self.SpeedWindow4.SetTicks(ticks)
        self.SpeedWindow4.SetTicksColour(wx.WHITE)
        self.SpeedWindow4.SetTicksFont(wx.Font(10, wx.TELETYPE, wx.NORMAL, wx.BOLD))        
        self.SpeedWindow4.SetHandColour(wx.WHITE)
        self.SpeedWindow4.SetSpeedBackground(wx.BLACK)        
        self.SpeedWindow4.SetMiddleText("AIR °C")
        self.SpeedWindow4.SetMiddleTextColour(wx.WHITE)
        self.SpeedWindow4.SetMiddleTextFont(wx.Font(8, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.SpeedWindow4.DrawExternalArc(False)
        self.SpeedWindow4.SetHandColour(wx.WHITE)
        self.SpeedWindow4.SetShadowColour(wx.Colour(50, 50, 50))  
        icon = wx.Icon("temp.ico", wx.BITMAP_TYPE_ICO)
        icon.SetWidth(12)
        icon.SetHeight(8)  
        self.SpeedWindow4.SetMiddleIcon(icon)                
        self.SpeedWindow4.SetSpeedValue(40)

        # Fifth SpeedMeter: We Use The Following Styles:
       
        self.SpeedWindow5 = SM.SpeedMeter(panel2,
                                          extrastyle=SM.SM_DRAW_HAND |
                                          SM.SM_DRAW_SECTORS |
                                          SM.SM_DRAW_SECONDARY_TICKS |
                                          SM.SM_DRAW_MIDDLE_TEXT |
                                SM.SM_DRAW_PARTIAL_FILLER |
                                          SM.SM_ROTATE_TEXT
                                
                                          )
                                
        self.SpeedWindow5.SetAngleRange(-pi/6, 7*pi/6)
        intervals = range(0, 61, 10)
        self.SpeedWindow5.SetIntervals(intervals)
        colours = [wx.BLACK]*4
        colours.append(wx.Colour(128, 128, 128))
        colours.append(wx.RED)
        self.SpeedWindow5.SetIntervalColours(colours)
        ticks = [str(interval) for interval in intervals]
        self.SpeedWindow5.SetTicks(ticks)
        self.SpeedWindow5.SetTicksColour(wx.WHITE)
        self.SpeedWindow5.SetTicksFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))      
        self.SpeedWindow5.SetNumberOfSecondaryTicks(4)
        self.SpeedWindow5.SetHandColour(wx.Colour(255, 50, 0))      
        self.SpeedWindow5.SetArcColour(wx.BLUE)
        self.SpeedWindow5.SetSpeedBackground(wx.BLACK)        
        self.SpeedWindow5.SetHandColour(wx.WHITE)
        self.SpeedWindow5.DrawExternalArc(False)
        self.SpeedWindow5.SetMiddleText("RPM x 100")
        self.SpeedWindow5.SetMiddleTextColour(wx.WHITE)
        self.SpeedWindow5.SetMiddleTextFont(wx.Font(9, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.SpeedWindow5.SetSpeedValue(5.6)
        

        # Sixth SpeedMeter: That Is Complete And Complex Example.
        #                   We Use The Following Styles:
        #
        # SM_DRAW_HAND: We Want To Draw The Hand (Arrow) Indicator
        # SM_DRAW_PARTIAL_FILLER: The Region Passed By The Hand Indicator Is Highlighted
        #                         With A Different Filling Colour
        # SM_DRAW_MIDDLE_ICON: We Draw An Icon In The Center Of SpeedMeter
        # SM_DRAW_GRADIENT: A Circular Colour Gradient Is Drawn Inside The SpeedMeter, To
        #                   Give Some Kind Of Scenic Effect
        # SM_DRAW_FANCY_TICKS: We Use wx.lib.
        # SM_DRAW_SHADOW: A Shadow For The Hand Indicator Is Drawn
        
        self.SpeedWindow6 = SM.SpeedMeter(panel6,
                                          extrastyle=SM.SM_DRAW_HAND |
                                          SM.SM_DRAW_SECTORS |
                                 SM.SM_DRAW_MIDDLE_TEXT |
                                          SM.SM_DRAW_MIDDLE_ICON,
                                          mousestyle=SM.SM_MOUSE_TRACK
                                          )

        self.SpeedWindow6.SetAngleRange(-pi/6, 7*pi/6)

        intervals = range(20, 121, 10)
        self.SpeedWindow6.SetIntervals(intervals)
        colours = [wx.BLUE]*2
        colours.extend([wx.BLACK]*7)
        colours.append(wx.RED)
        self.SpeedWindow6.SetIntervalColours(colours)
        ticks = [str(interval) + "°" for interval in intervals]
        self.SpeedWindow6.SetTicks(ticks)
        self.SpeedWindow6.SetTicksColour(wx.WHITE)
        self.SpeedWindow6.SetTicksFont(wx.Font(10, wx.TELETYPE, wx.NORMAL, wx.BOLD))        
        self.SpeedWindow6.SetHandColour(wx.WHITE)
        self.SpeedWindow6.SetSpeedBackground(wx.BLACK)
        self.SpeedWindow6.SetArcColour(wx.BLUE)
        self.SpeedWindow6.DrawExternalArc(False)
        self.SpeedWindow6.SetHandColour(wx.WHITE)
        self.SpeedWindow6.SetMiddleText("OIL °C")
        self.SpeedWindow6.SetMiddleTextColour(wx.WHITE)
        self.SpeedWindow6.SetMiddleTextFont(wx.Font(8, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.SpeedWindow6.SetMiddleIcon(icon) 
        self.SpeedWindow6.SetSpeedValue(40)
        # End Of SpeedMeter Controls Construction. Add Some Functionality

        # These Are Cosmetics For The First to 6th SpeedMeter Control
        bsizer1 = wx.BoxSizer(wx.VERTICAL)
        hsizer1 = wx.BoxSizer(wx.HORIZONTAL)        
        bsizer1.Add(self.SpeedWindow1, 1, wx.EXPAND)
        bsizer1.Add(hsizer1, 0, wx.EXPAND)
        panel1.SetSizer(bsizer1)
        bsizer2 = wx.BoxSizer(wx.VERTICAL)
        hsizer2 = wx.BoxSizer(wx.HORIZONTAL)
        bsizer2.Add(self.SpeedWindow2, 1, wx.EXPAND)        
        bsizer2.Add(hsizer2, 0, wx.EXPAND)        
        panel2.SetSizer(bsizer2)
        bsizer3 = wx.BoxSizer(wx.VERTICAL)        
        hsizer3 = wx.BoxSizer(wx.HORIZONTAL)                
        bsizer3.Add(self.SpeedWindow3, 1, wx.EXPAND)
        bsizer3.Add(hsizer3, 0, wx.EXPAND)
        panel3.SetSizer(bsizer3)
        bsizer4 = wx.BoxSizer(wx.VERTICAL)
        hsizer4 = wx.BoxSizer(wx.HORIZONTAL)                      
        bsizer4.Add(self.SpeedWindow4, 1, wx.EXPAND)
        bsizer4.Add(hsizer4, 0, wx.EXPAND)
        panel4.SetSizer(bsizer4)
        bsizer5 = wx.BoxSizer(wx.VERTICAL)
        hsizer5 = wx.BoxSizer(wx.HORIZONTAL)                    
        bsizer5.Add(self.SpeedWindow5, 1, wx.EXPAND)
        bsizer5.Add(hsizer5, 0, wx.EXPAND)
        panel5.SetSizer(bsizer5)
        bsizer6 = wx.BoxSizer(wx.VERTICAL)
        hsizer6 = wx.BoxSizer(wx.HORIZONTAL)
        bsizer6.Add(self.SpeedWindow6, 1, wx.EXPAND)
        bsizer6.Add(hsizer6, 0, wx.EXPAND)
        panel6.SetSizer(bsizer6)
        
        sizer.Add(panel1, 1, wx.EXPAND)
        sizer.Add(panel2, 1, wx.EXPAND)
        sizer.Add(panel3, 1, wx.EXPAND)
        sizer.Add(panel4, 1, wx.EXPAND)
        sizer.Add(panel5, 1, wx.EXPAND)
        sizer.Add(panel6, 1, wx.EXPAND)

        sizer.AddGrowableRow(0)
        sizer.AddGrowableRow(1)
        sizer.AddGrowableCol(0)
        sizer.AddGrowableCol(1)
        sizer.AddGrowableCol(2)
        
        panel.SetSizer(sizer)
        sizer.Layout()
        
if __name__ == "__main__":
    
    app = wx.PySimpleApp()
    frame = SpeedMeterDemo()
    frame.Show()
    frame.Maximize()

    app.MainLoop()

   
