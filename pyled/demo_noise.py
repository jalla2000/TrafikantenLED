#!/usr/bin/env python
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
display.flush()
display.send()
while True:
    display.flush()
    for row in range(0, 8*DISPLAY_LINES):
        for col in range(0, 32):
            display.setByte(row, col, random.randint(0, 255))
    for i in range(0, DISPLAY_LINES):
        display.send(i)

