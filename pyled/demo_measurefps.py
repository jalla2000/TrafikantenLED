#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import time
from ledfont import LedFont
from leddisplay import LedDisplay

DISPLAY_LINES = 4
DEVICE = '/dev/ttyUSB0'

font = LedFont()
display = LedDisplay(font, lines=DISPLAY_LINES, device=DEVICE)

import random
counter = 1
snowflakes = []

def moveSnowflakes(flake):
    flake[0] += 1
    if flake[0] > 127:
        flake[0] = 0
    flake[1] += 1
    if flake[1] > 31:
        flake[1] = 0

for i in range(0, 30):
    snowflakes.append([random.randint(0, 127), random.randint(0, 31)])

tick = 0.0
fps = 0.0
while True:
    counter += 1
    display.flush()
    for flake in snowflakes:
        moveSnowflakes(flake)
        color = random.randint(0, 2)
        if color == 0: color = 'green'
        if color == 1: color = 'red'
        if color == 2: color = 'orange'
        display.setPixel(flake[0], flake[1], color)
    display.writeTxt("Frame: %s" % str(counter), color='green', xpos=30, ypos=12)
    if (counter % 10 == 0):
        now = time.time()
        fps = 1.0/(now - tick)*10
        tick = now
    display.writeTxt("FPS: %s" % str(fps)[:5], color='green', xpos=30, ypos=20)
    display.send()

