#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import time
import sys
from ledfont import LedFont
from leddisplay import LedDisplay
import random

DISPLAY_LINES = 4
DEVICE = '/dev/ttyUSB0'
font = LedFont()
display = LedDisplay(font, lines=DISPLAY_LINES, device=DEVICE)

display.flush(DISPLAY_LINES-1)
display.send()
display.writeTxt("Line 1", xpos=0, ypos=0, color='red')
display.writeTxt("Line 2", xpos=0, ypos=8, color='green')
display.writeTxt("Line 3", xpos=0, ypos=16, color='orange')
display.writeTxt("Line 4", xpos=0, ypos=24, color='green')
display.send()
time.sleep(0.5)
for i in range(100, -1, -4):
    display.flush(DISPLAY_LINES-1)
    display.writeTxt("Scrolling", xpos=i, ypos=(DISPLAY_LINES-1)*8, color='green')
    display.send(DISPLAY_LINES-1)


alice           = [[ 0b00000000, 0b00111100, 0b00000000 ],
                   [ 0b00000000, 0b11111111, 0b00000000 ],
                   [ 0b00000011, 0b11111111, 0b11000000 ],
                   [ 0b00001111, 0b00111100, 0b11110000 ],
                   [ 0b00001111, 0b11111111, 0b11110000 ],
                   [ 0b00000000, 0b11000011, 0b00000000 ],
                   [ 0b00000011, 0b00111100, 0b11000000 ],
                   [ 0b00001100, 0b11000011, 0b00110000 ],
                   [ 0b00000000, 0b00000000, 0b00000000 ],
                   [ 0b00000000, 0b00000000, 0b00000000 ]]

bob             = [[ 0b00001100, 0b00000000, 0b11000000 ],
                   [ 0b00000011, 0b00000011, 0b00000000 ],
                   [ 0b00001111, 0b11111111, 0b11000000 ],
                   [ 0b00111100, 0b11111100, 0b11110000 ],
                   [ 0b11111111, 0b11111111, 0b11111100 ],
                   [ 0b11001111, 0b11111111, 0b11001100 ],
                   [ 0b11001100, 0b00000000, 0b11001100 ],
                   [ 0b00000011, 0b11001111, 0b00000000 ],
                   [ 0b00000000, 0b00000000, 0b00000000 ],
                   [ 0b00000000, 0b00000000, 0b00000000 ]]

carol           = [[ 0b00000000, 0b00001100, 0b00000000, 0b00000000 ],
                   [ 0b00000000, 0b00111111, 0b00000000, 0b00000000 ],
                   [ 0b00000000, 0b00111111, 0b00000000, 0b00000000 ],
                   [ 0b00000000, 0b00111111, 0b00000000, 0b00000000 ],
                   [ 0b00111111, 0b11111111, 0b11111111, 0b00000000 ],
                   [ 0b11111111, 0b11111111, 0b11111111, 0b11000000 ],
                   [ 0b11111111, 0b11111111, 0b11111111, 0b11000000 ],
                   [ 0b11111111, 0b11111111, 0b11111111, 0b11000000 ],
                   [ 0b11111111, 0b11111111, 0b11111111, 0b11000000 ],
                   [ 0b00000000, 0b00000000, 0b00000000, 0b00000000 ]]

class Thing:

    def __init__(self, xpos, ypos, sprite, color):
        self.sprite = sprite
        self.xpos = xpos
        self.ypos = ypos
        self.color = color

scene = []
for i in range(0, 4):
    scene.append(Thing(5+i*15, 2, bob, 'red'))
for i in range(0, 4):
    scene.append(Thing(5+i*15, 12, alice, 'green'))

scene.append(Thing(50, 20, carol, 'orange'))

direction = 'right'

while True:
    display.flush()
    turn = False
    for thing in scene:
        display.drawSprite(thing.sprite, thing.xpos, thing.ypos, thing.color)
        if direction == 'right':
            thing.xpos += 1
        else:
            thing.xpos -= 1
        if thing.xpos > 115 or  thing.xpos < 3:
            turn = True

    if turn:
        if direction == 'right':
            direction = 'left'
        else:
            direction = 'right'

    display.send()
#    time.sleep(0.1)
