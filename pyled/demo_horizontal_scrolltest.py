#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import urllib2
import time
import json
import sys
from ledfont import LedFont
from leddisplay import LedDisplay
import random

font = LedFont()
display = LedDisplay(font, lines=4, device="/dev/ttyUSB0")

color = 'orange'
while True:
    for i in range(0, -107, -1):
        y = 0
        x = i
        display.flush()
        display.writeTxt("LL 20 Galgeberg 8min  LL 20 Galgeberg 8min", xpos=x, ypos=0+y, color=color)
        display.writeTxt("LL 20 Galgeberg 8min  LL 20 Galgeberg 8min", xpos=x, ypos=8+y, color=color)
        display.writeTxt("LL 20 Galgeberg 8min  LL 20 Galgeberg 8min", xpos=x, ypos=16+y, color=color)
        display.writeTxt("LL 20 Galgeberg 8min  LL 20 Galgeberg 8min", xpos=x, ypos=24+y, color=color)
        display.send()
