#!/usr/bin/env python
# -*- coding: utf-8 -*-
from leddisplay import LedDisplay
from ledfont import LedFont
import time

font = LedFont()
display = LedDisplay(font)

for i in range(0, 32) + range(32, 0, -1):
    display.flush()
    display.writeTxt('NUCCC 2011', ypos=i, xpos=i)
    display.send()

time.sleep(1)
