#!/usr/bin/python

import os
import pygame
import time
import math

pygame.display.init()

# get screen resolution and display fullscreen
size = (pygame.display.Info().current_w, pygame.display.Info().current_h)
print "Framebuffer size: %d x %d" % (size[0], size[1])

screen = pygame.display.set_mode(size, pygame.FULLSCREEN)

pygame.font.init()
font = pygame.font.Font(None, 30)

drawColor = (255, 255,255)  # White
backColor = (0, 0, 0)  # Black

needlesize = 120

centerX = 200
centerY = 200

# write out value labels in 180 arc
for x in (0,30,60,90,120,150,180):
            x = math.radians(-90-x) 
            text_surface = font.render('%s' % x, True, drawColor)
            screen.blit(text_surface, (centerX -10 + math.sin(x) * needlesize, centerY + math.cos(x) *  needlesize))
            pygame.display.update()

# Smaller radius so we don't  go over the labels with the needle:

needlesize = 90

# Show a sweeping needle
for x in range(0,180):
            x = math.radians(-90-x)

            pygame.draw.line(screen, drawColor, (centerX, centerY), (centerX + math.sin(x) * needlesize, centerY + math.cos(x) * needlesize))
            pygame.display.update()

            # erase line by drawing in black
            pygame.draw.line(screen, backColor, (centerX, centerY), (centerX + math.sin(x) * needlesize, centerY + math.cos(x) * needlesize)) 

# Wait 2 seconds
time.sleep(2)
